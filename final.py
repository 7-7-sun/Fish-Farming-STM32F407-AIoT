import os
from maix import image, display, app, camera, http, nn
import cv2
import numpy as np
import paho.mqtt.client as mqtt
import time
import json
from threading import Thread, Lock
import requests

# ===================== 全局配置 =====================
# MQTT 配置
MQTT_BROKER = "139.9.0.201"
MQTT_PORT = 1883
MQTT_USERNAME = os.environ.get("MQTT_USERNAME", "admin")
MQTT_PASSWORD = os.environ.get("MQTT_PASSWORD", "")
MQTT_TOPIC_WATER = "fish/identify/water"   # 水位主题
MQTT_TOPIC_COUNT = "fish/identify/count"   # 计数主题

# 推流服务器配置
UPLOAD_URL = "http://139.9.0.201:9000/upload"

# 水位检测配置
STANDARD_WATER_LINE_Y = 200
last_water = 200
WATER_SEARCH_MIN_Y = 150  # 水位搜索区间底部
WATER_SEARCH_MAX_Y = 280  # 水位搜索区间顶部

# 鱼类检测 感兴趣区间（和水位检测格式一致）
FISH_MIN_Y = 160   # 鱼类检测最低Y
FISH_MAX_Y = 320   # 鱼类检测最高Y

# 全局变量
water_data_json = ""
count_data_json = ""
data_lock = Lock()
client = mqtt.Client()

# HTTP 流页面
html = """<!DOCTYPE html><html><body><img src="/stream" style="width:320px;height:240;"></body></html>"""

# ===================== MQTT 相关 =====================
def on_connect(client, userdata, flags, rc):
    if rc == 0: 
        print("✅ MQTT 连接成功")
    else:
        print(f"⚠️ MQTT 连接失败，错误码：{rc}")

def send_mqtt_water():
    """发送水位MQTT数据"""
    while not app.need_exit():
        with data_lock:
            payload = water_data_json
        if payload:
            try:
                client.publish(MQTT_TOPIC_WATER, payload)
            except Exception as e:
                print(f"水位MQTT发送失败: {e}")
        time.sleep(0.5)

def send_mqtt_count():
    """发送计数MQTT数据"""
    while not app.need_exit():
        with data_lock:
            payload = count_data_json
        if payload:
            try:
                client.publish(MQTT_TOPIC_COUNT, payload)
            except Exception as e:
                print(f"计数MQTT发送失败: {e}")
        time.sleep(1)

# ===================== 水位检测 =====================
def detect_water(img_gray):
    global last_water
    h, w = img_gray.shape

    # 截取仅包含水面的小区域分析
    search_region = img_gray[WATER_SEARCH_MIN_Y:WATER_SEARCH_MAX_Y, :]
    if search_region.shape[0] == 0:
        return last_water

    # 计算行平均亮度和亮度突变
    row_mean = np.mean(search_region, axis=1)
    diff = np.abs(np.diff(row_mean))

    # 找出强亮度突变点
    peaks = np.where(diff > np.mean(diff) * 1.8)[0]
    if len(peaks) == 0:
        return last_water

    best_y = last_water
    min_dist = 9999

    for rel_y in peaks:
        abs_y = rel_y + WATER_SEARCH_MIN_Y
        # 校验：空气(亮) -> 水(暗) 的下降沿
        if row_mean[rel_y] > row_mean[rel_y + 1] and 50 < row_mean[rel_y + 1] < 180:
            dist = abs(abs_y - last_water)
            if dist < min_dist:
                best_y = abs_y
                min_dist = dist

    last_water = best_y
    return int(best_y)

# ===================== 主程序 =====================
def main():
    global water_data_json, count_data_json

    # 初始化摄像头（兼容模型输入尺寸）
    detector = nn.YOLO11(model="/root/models/best_int8.mud", dual_buff=True)
    cam = camera.Camera(detector.input_width(), detector.input_height(), detector.input_format())
    disp = display.Display()

    # MQTT 初始化
    try:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
        client.on_connect = on_connect
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_start()
        # 启动MQTT发送线程
        Thread(target=send_mqtt_water, daemon=True).start()
        Thread(target=send_mqtt_count, daemon=True).start()
    except Exception as e:
        print(f"⚠️ MQTT初始化失败: {e}")

    # HTTP 流初始化
    stream = http.JpegStreamer()
    stream.set_html(html)
    stream.start()

    print(f"推流目标: {UPLOAD_URL}")

    while not app.need_exit():
        # 1. 读取摄像头图像
        img_raw = cam.read()
        img_cv = image.image2cv(img_raw)
        img_maix = img_raw  # maix原生图像对象（用于NN检测和显示）

        # 2. 水位检测
        gray = cv2.cvtColor(img_cv, cv2.COLOR_BGR2GRAY)
        real_y = detect_water(gray)
        low_water = real_y > STANDARD_WATER_LINE_Y + 4

        # 3. 鱼类识别计数 + 感兴趣区间过滤
        bm_count = 0
        gf_count = 0
        bm_top_ys = []
        gf_top_ys = []
        objs = detector.detect(img_maix, 0.5, 0.5)

        # 绘制检测框和统计数量（只统计在感兴趣区间内的目标）
        for obj in objs:
            # ===================== 核心修改：鱼类检测感兴趣区间过滤 =====================
            # 只保留 Y 坐标在 160~320 之间的目标
            if not (FISH_MIN_Y <= obj.y <= FISH_MAX_Y):
                continue  # 不在区间内，直接跳过
            # ==========================================================================
            
            label = detector.labels[obj.class_id]
            img_maix.draw_rect(obj.x, obj.y, obj.w, obj.h, color=image.COLOR_RED)
            msg = f'{label}: {obj.score:.2f}'
            img_maix.draw_string(obj.x, obj.y, msg, color=image.COLOR_RED)

            if label == "BM":
                bm_count += 1
                bm_top_ys.append(obj.y)
            elif label == "GF":
                gf_count += 1
                gf_top_ys.append(obj.y)

        # 4. 可视化绘制（水位线 + 状态文本）
        # 转换回CV格式绘制水位线（兼容坐标）
        img_cv = image.image2cv(img_maix)
        cv2.line(img_cv, (0, STANDARD_WATER_LINE_Y), (img_cv.shape[1], STANDARD_WATER_LINE_Y), (0,255,0), 2)
        cv2.line(img_cv, (0, real_y), (img_cv.shape[1], real_y), (0,0,255), 2)
        
        # 水位状态文本
        water_text = "LOW WATER" if low_water else "WATER NORMAL"
        water_color = (0,0,255) if low_water else (0,255,0)
        cv2.putText(img_cv, water_text, (15,30), cv2.FONT_HERSHEY_SIMPLEX, 1, water_color, 2)
        
        
        # 转换回maix图像对象用于显示
        img_maix = image.cv2image(img_cv)

        # 5. 显示和推流
        disp.show(img_maix)
        stream.write(img_maix.to_jpeg())

        # 6. 推流到服务器
        try:
            jpg_bytes = img_maix.to_jpeg().to_bytes()
            requests.post(UPLOAD_URL, data=jpg_bytes, timeout=2)
        except Exception as e:
            print(f"推流失败: {e}")

        # 7. 更新MQTT数据
        with data_lock:
            # 水位数据
            water_data = {
                "standard": STANDARD_WATER_LINE_Y,
                "real": int(real_y),
                "status": "low" if low_water else "normal"
            }
            water_data_json = json.dumps(water_data)

            # 计数数据
            count_data = {
                'bm_count': bm_count,
                'gf_count': gf_count,
                'bm_top_ys': bm_top_ys,
                'gf_top_ys': gf_top_ys
            }
            count_data_json = json.dumps(count_data)

if __name__ == "__main__":
    main()