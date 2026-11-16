# 鱼菜共生物联网系统（Fish-Farming-STM32F407-AIoT）

基于 STM32F407 的鱼菜共生 / 水产养殖物联网项目，包含下位机固件、Vue 3 管理端和 MaixPy 视觉识别脚本。

## 目录结构

- `STM32F407_ProjectTemplate/`：STM32F407 固件与 Keil MDK 工程
- `FishFrontEnd/`：Vue 3 + Vite 前端管理端
- `final.py`：MaixPy 视觉识别与水情上报脚本
- `sketch_apr15a.ino`：Arduino 端示例程序

## 快速开始

### 固件

使用 Keil MDK 打开 `STM32F407_ProjectTemplate/STM32F407_ProjectTemplate/project/MDK(V5)/Project.uvprojx`，编译并下载到 STM32F407 开发板。

### 前端

```bash
cd FishFrontEnd
npm install
cp .env.example .env
npm run dev
```

### 视觉脚本

```bash
pip install paho-mqtt requests opencv-python numpy
export MQTT_USERNAME=your_user
export MQTT_PASSWORD=your_password
python final.py
```

## 配置说明

MQTT 服务器地址在 `final.py` 中配置；账号和密码通过环境变量 `MQTT_USERNAME` / `MQTT_PASSWORD` 传入，请勿把真实密码提交到仓库。前端接口地址通过 `FishFrontEnd/.env` 中的 `VITE_API_BASE_URL` 配置。
`sketch_apr15a.ino` 顶部的 Wi-Fi 和 MQTT 账号密码为占位符，运行前请改为自己的配置。
