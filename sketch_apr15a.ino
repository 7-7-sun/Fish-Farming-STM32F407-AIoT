#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h> 

// FreeRTOS 头文件 (ESP32 默认自带)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// ==================== 1. Wi-Fi 和 MQTT 配置 ====================
// 请填入自己的 Wi-Fi 和 MQTT 凭据，不要提交真实密码
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "139.9.0.201"; 
const int mqtt_port = 1883; 
const char* mqtt_user = "YOUR_MQTT_USER";     
const char* mqtt_password = "YOUR_MQTT_PASSWORD";

// ==================== 2. MQTT Topic 定义 ====================
const char* topicLight = "esp32/light";
const char* topicFeed1 = "fish/control/feed/1";
const char* topicFeed2 = "fish/control/feed/2";
const char* topicRecord1 = "fish/record/feed/1";
const char* topicRecord2 = "fish/record/feed/2";

const char* topic_pub_temp = "fish/sensor/temp";
const char* topic_pub_turb = "fish/sensor/turbidity";
const char* topic_pub_ph   = "fish/sensor/ph";
const char* topic_pub_do   = "fish/sensor/do";

const char* topic_pub_heater     = "fish/status/heater";
const char* topic_pub_pump_in    = "fish/status/pump_in";
const char* topic_pub_pump_out   = "fish/status/pump_out";
const char* topic_pub_air_pump   = "fish/status/air_pump";

const char* topic_sub_control = "fish/control/+"; 

// ==================== 3. 硬件引脚与参数定义 ====================
#define LED_PIN     48    
#define LED_COUNT   45    

// --- 电机 1 (对应投喂1) ---
#define MOTOR_A_PWM 13  
#define MOTOR_A_IN1 4   
#define MOTOR_A_IN2 5   
const int pwmChannelA = 0; 

// --- 电机 2 (对应投喂2) ---
#define MOTOR_B_PWM 14  
#define MOTOR_B_IN1 6   
#define MOTOR_B_IN2 7   
const int pwmChannelB = 1; 

const int pwmFreq = 5000;      
const int pwmResolution = 8;   

#define RXD2 16
#define TXD2 17

// ==================== 4. 全局对象与 FreeRTOS 变量 ====================
WiFiClient espClient;
PubSubClient client(espClient);
HardwareSerial SerialMCU(2); 
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// 多线程共享变量需要加 volatile 修饰，防止编译器过度优化导致数据不同步
volatile int countdown1 = 0, countdown2 = 0;
volatile float feedTime1 = 0.0f, feedTime2 = 0.0f;
volatile unsigned long lastCheck1 = 0, lastCheck2 = 0;
volatile bool startFeed1 = false, startFeed2 = false;

// 声明互斥锁
SemaphoreHandle_t mqttMutex;

// ==================== 5. 基础硬件控制函数 ====================

void setAllLedColor(int r, int g, int b){
  for(int i=0; i<LED_COUNT; i++){
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void lightInit() {
  pixels.begin();
  pixels.setBrightness(80);
  setAllLedColor(255, 220, 180); 
}

void motorInit() {
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);

  ledcSetup(pwmChannelA, pwmFreq, pwmResolution); 
  ledcAttachPin(MOTOR_A_PWM, pwmChannelA);  
  
  ledcSetup(pwmChannelB, pwmFreq, pwmResolution); 
  ledcAttachPin(MOTOR_B_PWM, pwmChannelB);  
  
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  ledcWrite(pwmChannelA, 0);

  digitalWrite(MOTOR_B_IN1, LOW);
  digitalWrite(MOTOR_B_IN2, LOW);
  ledcWrite(pwmChannelB, 0);
}

void motorRun(int motorID, bool forward, int speed) {
  speed = constrain(speed, 0, 255);
  if (motorID == 1) {
    if (speed == 0) {
      digitalWrite(MOTOR_A_IN1, LOW);
      digitalWrite(MOTOR_A_IN2, LOW);
    } else {
      digitalWrite(MOTOR_A_IN1, forward ? HIGH : LOW);
      digitalWrite(MOTOR_A_IN2, forward ? LOW : HIGH);
    }
    ledcWrite(pwmChannelA, speed);
  } else if (motorID == 2) {
    if (speed == 0) {
      digitalWrite(MOTOR_B_IN1, LOW);
      digitalWrite(MOTOR_B_IN2, LOW);
    } else {
      digitalWrite(MOTOR_B_IN1, forward ? HIGH : LOW);
      digitalWrite(MOTOR_B_IN2, forward ? LOW : HIGH);
    }
    ledcWrite(pwmChannelB, speed);
  }
}

// ==================== 6. 网络与 MQTT 通信 ====================

void setup_wifi() {
  Serial.print("Connecting WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void parseFeedMsg(String msg, volatile int &cd, volatile float &ft) {
  int idx1 = msg.indexOf(",");
  if(idx1 == -1) return;
  String s1 = msg.substring(0, idx1);
  String s2 = msg.substring(idx1+1);
  cd = s1.substring(s1.indexOf(":")+1).toInt();
  ft = s2.substring(s2.indexOf(":")+1).toFloat();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) { msg += (char)payload[i]; }
  msg.trim();
  String t = String(topic);

  Serial.printf("[MQTT 下发] Topic: %s | Msg: %s\n", topic, msg.c_str());

  if (t == topicFeed1) {
    parseFeedMsg(msg, countdown1, feedTime1);
    startFeed1 = true;
    lastCheck1 = millis();
    return;
  }
  if (t == topicFeed2) {
    parseFeedMsg(msg, countdown2, feedTime2);
    startFeed2 = true;
    lastCheck2 = millis();
    return;
  }

  if (t == topicLight) {
    if(msg.equalsIgnoreCase("ON")) setAllLedColor(255, 220, 180);
    else if(msg.equalsIgnoreCase("OFF")) setAllLedColor(0, 0, 0);
    else {
      int c1 = msg.indexOf(',');
      int c2 = msg.indexOf(',', c1+1);
      if(c1 != -1 && c2 != -1) setAllLedColor(msg.substring(0, c1).toInt(), msg.substring(c1+1, c2).toInt(), msg.substring(c2+1).toInt());
    }
    return;
  }

  if (t.startsWith("fish/control/") && t != topicFeed1 && t != topicFeed2) {
    String cmdStatus = (msg == "true") ? "1" : "0";
    if (t == "fish/control/heater") SerialMCU.printf("CMD_HEATER:%s\n", cmdStatus.c_str());
    else if (t == "fish/control/pump_in") SerialMCU.printf("CMD_PUMP_IN:%s\n", cmdStatus.c_str());
    else if (t == "fish/control/pump_out") SerialMCU.printf("CMD_PUMP_OUT:%s\n", cmdStatus.c_str());
    else if (t == "fish/control/air_pump") SerialMCU.printf("CMD_AIR_PUMP:%s\n", cmdStatus.c_str());
  }
}

// 封装一个带锁的 Publish 函数，确保多线程安全
void safeMqttPublish(const char* topic, const char* payload) {
  if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(1000)) == pdTRUE) { // 等待锁，最多等1秒
    if(client.connected()) {
      client.publish(topic, payload);
    }
    xSemaphoreGive(mqttMutex); // 发送完释放锁
  } else {
    Serial.println("[警告] 无法获取 MQTT 锁，丢弃消息");
  }
}

// ==================== 7. FreeRTOS 任务定义 ====================

// 任务1：网络守护与 MQTT 心跳 (最高优先级，确保持续在线)
void TaskNetwork(void *pvParameters) {
  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      setup_wifi();
    } else if (!client.connected()) {
      String clientId = "ESP32-FishGateway-" + String(random(0xffff), HEX);
      Serial.print("Connecting MQTT... ");
      // 这里的 connect 不涉及共享发送，且如果断网其他线程的发布本就会失败，所以不强制加锁
      if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
        Serial.println("成功!");
        client.subscribe(topicFeed1);
        client.subscribe(topicFeed2);
        client.subscribe(topicLight);
        client.subscribe(topic_sub_control);
      } else {
        vTaskDelay(pdMS_TO_TICKS(5000));
      }
    } else {
      // 极其关键：拿锁执行 MQTT 心跳收发
      if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        client.loop();
        xSemaphoreGive(mqttMutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // 喂看门狗
  }
}

// 任务2：STM32 串口监听 (持续运行)
void TaskSTM32(void *pvParameters) {
  for (;;) {
    while (SerialMCU.available()) {
      String incomingData = SerialMCU.readStringUntil('\n');
      incomingData.trim();
      if (incomingData.length() == 0) continue;

      // ==========================================
      // 串口输出STM32数据：
      Serial.println("[STM32 数据] " + incomingData);
      // ==========================================

      int colonIndex = incomingData.indexOf(':');
      if (colonIndex > 0) {
        String key = incomingData.substring(0, colonIndex);
        String value = incomingData.substring(colonIndex + 1);

        // 使用安全的带锁发布
        if (key == "TEMP") safeMqttPublish(topic_pub_temp, value.c_str());
        else if (key == "TURB") safeMqttPublish(topic_pub_turb, value.c_str());
        else if (key == "PH") safeMqttPublish(topic_pub_ph, value.c_str());
        else if (key == "DO_CONC") safeMqttPublish(topic_pub_do, value.c_str());
        else if (key == "HEAT") safeMqttPublish(topic_pub_heater, value == "1" ? "true" : "false");
        else if (key == "ADD_PUMP") safeMqttPublish(topic_pub_pump_in, value == "1" ? "true" : "false");
        else if (key == "DRAIN_PUMP") safeMqttPublish(topic_pub_pump_out, value == "1" ? "true" : "false");
        else if (key == "AIR_PUMP") safeMqttPublish(topic_pub_air_pump, value == "1" ? "true" : "false");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20)); // 让出 CPU 资源
  }
}

// 任务3：喂食控制器 (修正为毫秒级响应)
void TaskFeed(void *pvParameters) {
  for (;;) {
    unsigned long now = millis();

    // =============== 电机 1 投喂逻辑 ===============
    if (startFeed1) {
      if (countdown1 <= 0) {
        // 核心修复：一旦倒计时为 0，不再等待，直接干活！
        Serial.println("电机1 开始喂食 (立即响应)!");
        motorRun(1, true, 200); 
        
        vTaskDelay(pdMS_TO_TICKS(feedTime1 * 1000)); 
        
        motorRun(1, true, 0);   
        safeMqttPublish(topicRecord1, String(feedTime1).c_str());
        startFeed1 = false; // 任务结束，复位标记
      } 
      else if (now - lastCheck1 >= 60000UL) {
        // 只有在真的需要倒计时（>0）时，才执行 60 秒扣减逻辑
        lastCheck1 = now;
        countdown1--;
        Serial.printf("投喂1 倒计时还剩: %d min\n", countdown1);
      }
    }

    // =============== 电机 2 投喂逻辑 ===============
    if (startFeed2) {
      if (countdown2 <= 0) {
        Serial.println("电机2 开始喂食 (立即响应)!");
        motorRun(2, true, 200); 
        
        vTaskDelay(pdMS_TO_TICKS(feedTime2 * 1000)); 
        
        motorRun(2, true, 0);   
        safeMqttPublish(topicRecord2, String(feedTime2).c_str());
        startFeed2 = false;
      } 
      else if (now - lastCheck2 >= 60000UL) {
        lastCheck2 = now;
        countdown2--;
        Serial.printf("投喂2 倒计时还剩: %d min\n", countdown2);
      }
    }
    
    // 将原本的 100ms 延迟缩短为 20ms，让按键触发手感更丝滑，完全不影响系统性能
    vTaskDelay(pdMS_TO_TICKS(20)); 
  }
}
// ==================== 8. 主程序 (分配任务) ====================

void setup() {
  Serial.begin(115200);
  SerialMCU.begin(115200, SERIAL_8N1, RXD2, TXD2); 
  delay(1000);
  Serial.println("\nESP32 智能鱼缸 (FreeRTOS 架构版) 启动...");

  // 1. 初始化硬件
  lightInit();
  motorInit();
  
  // 2. 初始化网络
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // 3. 创建互斥锁
  mqttMutex = xSemaphoreCreateMutex();
  if (mqttMutex == NULL) {
    Serial.println("互斥锁创建失败，系统死机");
    while(1);
  }

  // 4. 创建 FreeRTOS 任务
  // xTaskCreate(任务函数, 任务名, 堆栈大小, 传递参数, 优先级(数字越大越先执行), 任务句柄)
  xTaskCreate(TaskNetwork, "NetworkTask", 4096, NULL, 3, NULL); // 网络任务
  xTaskCreate(TaskSTM32, "STM32Task", 4096, NULL, 2, NULL);   // 串口任务
  xTaskCreate(TaskFeed, "FeedTask", 2048, NULL, 1, NULL);     // 喂食控制
}

void loop() {
  // 引入 FreeRTOS 后，主循环不需要再执行任何实质性逻辑
  // 把它直接挂起并自我销毁，把 Core 1 的算力全部分给我们的 Task
  vTaskDelete(NULL); 
}