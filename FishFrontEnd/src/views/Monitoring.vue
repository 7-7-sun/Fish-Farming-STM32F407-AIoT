<template>
  <div class="monitoring-page">
    <!-- 顶部标题区 -->
    <div class="page-header">
      <div class="header-left">
        <h2 class="page-title">实时监控中心</h2>
        <p class="page-subtitle">水下摄像头硬件画面实时推流</p>
      </div>
      <div class="header-actions">
        <el-tag type="success" effect="dark" round class="live-tag">
          <span class="live-dot"></span> LIVE
        </el-tag>
      </div>
    </div>

    <!-- 监控主体 -->
    <el-row :gutter="24">
      <el-col :span="18">
        <el-card class="video-card" shadow="never" :body-style="{ padding: '0px' }">
          <div class="video-container" ref="videoContainerRef" :class="{ loading: isLoading }">
            <div class="video-overlay">
              <div class="overlay-top">
                <div class="camera-info">
                  <el-icon class="mr-2"><VideoCamera /></el-icon>
                  <span>Camera A01 - 主视区</span>
                </div>
                <span class="timestamp">{{ currentTime }}</span>
              </div>
              <div class="overlay-bottom">
                <div class="controls">
                  <el-tooltip :content="isPlaying ? '暂停监控' : '恢复监控'" placement="top">
                    <el-button 
                      type="primary" 
                      circle 
                      :icon="isPlaying ? VideoPause : VideoPlay" 
                      @click="togglePlay" 
                      size="large"
                    />
                  </el-tooltip>
                  <el-tooltip content="全屏" placement="top">
                    <el-button circle :icon="FullScreen" @click="toggleFullScreen" size="large" />
                  </el-tooltip>
                </div>
              </div>
            </div>
            
            <!-- 视频流: 将后端的 /stream 直接作为 img 的 src -->
            <img
              v-if="isPlaying && !streamError"
              :src="streamUrl"
              class="video-stream"
              @error="handleStreamError"
              @load="isLoading = false"
              crossorigin="anonymous"
              alt="实时视频流"
            />
            
            <!-- 错误状态 -->
            <div v-else-if="streamError" class="video-error">
              <el-icon class="error-icon"><Warning /></el-icon>
              <p>视频流加载失败，请检查摄像头硬件连接或后端服务</p>
              <el-button type="primary" plain @click="retryStream">重试连接</el-button>
            </div>
            
            <!-- 暂停状态 -->
            <div v-else class="video-paused">
              <el-icon class="paused-icon"><VideoCamera /></el-icon>
              <p>监控已暂停</p>
              <el-button type="primary" @click="togglePlay">恢复监控</el-button>
            </div>
          </div>
        </el-card>
      </el-col>

      <el-col :span="6">
        <el-card class="info-card" shadow="never">
          <template #header>
            <div class="card-header">
              <el-icon><DataLine /></el-icon>
              <span>监控信道信息</span>
            </div>
          </template>
          
          <div class="info-list">
            <div class="info-item">
              <span class="label">流地址 (GET)</span>
              <el-tooltip :content="streamUrl" placement="left">
                <span class="value truncate">/stream</span>
              </el-tooltip>
            </div>
            <div class="info-item">
              <span class="label">推流地址 (POST)</span>
              <el-tooltip content="/upload" placement="left">
                <span class="value truncate">/upload</span>
              </el-tooltip>
            </div>
            <div class="info-item">
              <span class="label">分辨率</span>
              <span class="value">1080P (FHD)</span>
            </div>
            <div class="info-item">
              <span class="label">帧率</span>
              <span class="value">30 FPS</span>
            </div>
            <div class="info-item">
              <span class="label">网络延迟</span>
              <span class="value text-success">~12 ms</span>
            </div>
            <div class="info-item">
              <span class="label">运行状态</span>
              <span class="value" :class="streamError ? 'text-danger' : 'text-success'">
                {{ streamError ? '离线' : '正常' }}
              </span>
            </div>
          </div>

          <el-divider />

          <!-- 实时水质数据 -->
          <div class="prediction-panel">
            <div class="panel-header">
              <el-icon><DataLine /></el-icon>
              <span>实时水质</span>
              <el-tag
                v-if="latestData"
                type="success"
                size="small"
                class="status-tag"
              >
                实时
              </el-tag>
            </div>

            <div v-if="latestDataLoading && !latestData" class="prediction-loading">
              <el-skeleton :rows="3" animated />
            </div>

            <div v-if="latestData" class="prediction-content">
              <div class="prediction-time">
                记录时间: {{ formatRecordTime(latestData.recordTime) }}
              </div>

              <div class="prediction-metrics">
                <div class="metric-item">
                  <span class="metric-label">浊度</span>
                  <span class="metric-value">
                    {{ latestData.turbidity?.toFixed(2) }}
                  </span>
                  <span class="metric-unit">NTU</span>
                </div>
                <div class="metric-item">
                  <span class="metric-label">pH值</span>
                  <span class="metric-value">
                    {{ latestData.phValue?.toFixed(2) }}
                  </span>
                </div>
                <div class="metric-item">
                  <span class="metric-label">温度</span>
                  <span class="metric-value">
                    {{ latestData.temperature?.toFixed(2) }}
                  </span>
                  <span class="metric-unit">°C</span>
                </div>
                <div class="metric-item">
                  <span class="metric-label">溶解氧</span>
                  <span class="metric-value">
                    {{ latestData.dissolvedOxygen?.toFixed(2) }}
                  </span>
                  <span class="metric-unit">mg/L</span>
                </div>
              </div>

              <!-- 鱼类D值信息 -->
              <el-divider v-if="fishData" class="fish-divider" />
              <div v-if="fishData" class="fish-dvalue-section">
                <div class="fish-dvalue-header">
                  <el-icon><Food /></el-icon>
                  <span>鱼类饥饿状态</span>
                </div>
                <div class="fish-dvalue-metrics">
                  <div class="metric-item fish-metric">
                    <span class="metric-label">黑鱼D值</span>
                    <span class="metric-value" :class="getDValueClass(fishData.bmDValue)">
                      {{ fishData.bmDValue ?? '-' }}
                    </span>
                    <span class="metric-unit">px</span>
                  </div>
                  <div class="metric-item fish-metric">
                    <span class="metric-label">金鱼D值</span>
                    <span class="metric-value" :class="getDValueClass(fishData.gfDValue)">
                      {{ fishData.gfDValue ?? '-' }}
                    </span>
                    <span class="metric-unit">px</span>
                  </div>
                  <div class="metric-item fish-metric">
                    <span class="metric-label">黑鱼数量</span>
                    <span class="metric-value">{{ fishData.bmCount ?? 0 }}</span>
                    <span class="metric-unit">条</span>
                  </div>
                  <div class="metric-item fish-metric">
                    <span class="metric-label">金鱼数量</span>
                    <span class="metric-value">{{ fishData.gfCount ?? 0 }}</span>
                    <span class="metric-unit">条</span>
                  </div>
                </div>
                <div v-if="fishData.hungerStatus" class="hunger-status">
                  <el-tag :type="getHungerStatusType(fishData.hungerStatus)" size="small">
                    {{ getHungerStatusText(fishData.hungerStatus) }}
                  </el-tag>
                </div>
              </div>
            </div>

            <div v-if="!latestData && !latestDataLoading" class="prediction-empty">
              <el-icon><DataLine /></el-icon>
              <p>暂无实时数据</p>
              <el-button type="primary" plain size="small" @click="fetchLatestData">
                刷新数据
              </el-button>
            </div>
          </div>

          <el-divider />

          <!-- 设备控制面板 -->
          <div class="device-control-panel">
            <div class="panel-header">
              <el-icon><Switch /></el-icon>
              <span>设备控制</span>
              <el-tag
                v-if="!deviceLoading"
                :type="deviceStatus.heater || deviceStatus.airPump || deviceStatus.pumpIn || deviceStatus.pumpOut ? 'success' : 'info'"
                size="small"
                class="status-tag"
              >
                {{ deviceStatus.heater || deviceStatus.airPump || deviceStatus.pumpIn || deviceStatus.pumpOut ? '运行中' : '待机' }}
              </el-tag>
            </div>

            <div class="device-grid">
              <!-- 加热棒 -->
              <div class="device-item" @click="toggleHeater">
                <div class="device-icon" :class="{ active: deviceStatus.heater }">
                  <el-icon><Sunny /></el-icon>
                </div>
                <div class="device-info">
                  <span class="device-name">加热棒</span>
                  <span class="device-status" :class="{ active: deviceStatus.heater }">
                    {{ deviceStatus.heater ? '开启' : '关闭' }}
                  </span>
                </div>
              </div>

              <!-- 空气泵 -->
              <div class="device-item" @click="toggleAirPump">
                <div class="device-icon" :class="{ active: deviceStatus.airPump }">
                  <el-icon><WindPower /></el-icon>
                </div>
                <div class="device-info">
                  <span class="device-name">空气泵</span>
                  <span class="device-status" :class="{ active: deviceStatus.airPump }">
                    {{ deviceStatus.airPump ? '开启' : '关闭' }}
                  </span>
                </div>
              </div>

              <!-- 进水泵 -->
              <div class="device-item" @click="togglePumpIn">
                <div class="device-icon" :class="{ active: deviceStatus.pumpIn }">
                  <el-icon><Download /></el-icon>
                </div>
                <div class="device-info">
                  <span class="device-name">进水泵</span>
                  <span class="device-status" :class="{ active: deviceStatus.pumpIn }">
                    {{ deviceStatus.pumpIn ? '开启' : '关闭' }}
                  </span>
                </div>
              </div>

              <!-- 出水泵 -->
              <div class="device-item" @click="togglePumpOut">
                <div class="device-icon" :class="{ active: deviceStatus.pumpOut }">
                  <el-icon><Upload /></el-icon>
                </div>
                <div class="device-info">
                  <span class="device-name">出水泵</span>
                  <span class="device-status" :class="{ active: deviceStatus.pumpOut }">
                    {{ deviceStatus.pumpOut ? '开启' : '关闭' }}
                  </span>
                </div>
              </div>
            </div>

            <!-- 投喂控制 -->
            <div class="feed-control">
              <el-button
                type="primary"
                :icon="Food"
                @click="openFeedDialog"
                class="feed-btn"
              >
                手动投喂
              </el-button>
            </div>
          </div>

          <el-divider />

          <!-- 水质预测数据 -->
          <div class="prediction-panel">
            <div class="panel-header">
              <el-icon><TrendCharts /></el-icon>
              <span>水质预测</span>
              <el-tag
                v-if="predictionData"
                :type="getStatusType(predictionData.status)"
                size="small"
                class="status-tag"
              >
                {{ getStatusText(predictionData.status) }}
              </el-tag>
            </div>

            <div v-if="predictionLoading" class="prediction-loading">
              <el-skeleton :rows="3" animated />
            </div>

            <div v-else-if="predictionData" class="prediction-content">
              <div class="prediction-time">
                预测时间: {{ formatPredictionTime(predictionData.predictionTime) }}
              </div>

              <div class="prediction-metrics">
                <div class="metric-item">
                  <span class="metric-label">浊度</span>
                  <span class="metric-value" :class="{ 'text-danger': predictionData.alerts?.turbidity }">
                    {{ predictionData.predictions.turbidity?.toFixed(2) }}
                  </span>
                  <span class="metric-unit">NTU</span>
                </div>
                <div class="metric-item">
                  <span class="metric-label">pH值</span>
                  <span class="metric-value" :class="{ 'text-danger': predictionData.alerts?.phValue }">
                    {{ predictionData.predictions.phValue?.toFixed(2) }}
                  </span>
                </div>
                <div class="metric-item">
                  <span class="metric-label">温度</span>
                  <span class="metric-value">
                    {{ predictionData.predictions.temperature?.toFixed(2) }}
                  </span>
                  <span class="metric-unit">°C</span>
                </div>
                <div class="metric-item">
                  <span class="metric-label">溶解氧</span>
                  <span class="metric-value">
                    {{ predictionData.predictions.dissolvedOxygen?.toFixed(2) }}
                  </span>
                  <span class="metric-unit">mg/L</span>
                </div>
              </div>

              <!-- 预警信息已隐藏 -->
              <!--
              <div v-if="predictionData.alerts && Object.keys(predictionData.alerts).length > 0" class="prediction-alerts">
                <div class="alert-header">
                  <el-icon><WarningFilled /></el-icon>
                  <span>预警信息</span>
                </div>
                <div class="alert-list">
                  <div
                    v-for="(alert, key) in predictionData.alerts"
                    :key="key"
                    class="alert-item"
                  >
                    {{ alert }}
                  </div>
                </div>
              </div>
              -->

              <!-- 置信度已隐藏 -->
              <!-- <div class="prediction-confidence">
                置信度: {{ (predictionData.confidence * 100).toFixed(0) }}%
              </div> -->
            </div>

            <div v-else class="prediction-empty">
              <el-icon><TrendCharts /></el-icon>
              <p>暂无预测数据</p>
              <el-button type="primary" plain size="small" @click="fetchPredictionData">
                刷新数据
              </el-button>
            </div>
          </div>

          <!-- 云台控制已隐藏 -->
          <!-- <el-divider />

          <div class="action-panel">
            <h4>云台控制 (PTZ)</h4>
            <div class="ptz-controls">
              <div class="ptz-row">
                <el-button circle :icon="Top" />
              </div>
              <div class="ptz-row">
                <el-button circle :icon="Back" />
                <el-button circle :icon="Refresh" type="primary" plain />
                <el-button circle :icon="Right" />
              </div>
              <div class="ptz-row">
                <el-button circle :icon="Bottom" />
              </div>
            </div>
          </div> -->
        </el-card>
      </el-col>
    </el-row>
  </div>

  <!-- 投喂对话框 -->
  <el-dialog
    v-model="feedDialogVisible"
    title="手动投喂"
    width="400px"
  >
    <el-form :model="feedForm" label-width="100px">
      <el-form-item label="选择舵机">
        <el-radio-group v-model="feedForm.servoId">
          <el-radio-button :label="1">舵机 1</el-radio-button>
          <el-radio-button :label="2">舵机 2</el-radio-button>
        </el-radio-group>
      </el-form-item>
      <el-form-item label="倒计时(分钟)">
        <el-input-number v-model="feedForm.countdown" :min="0" :max="60" />
        <span class="form-tip">0表示立即投喂</span>
      </el-form-item>
      <el-form-item label="投喂时长(秒)">
        <el-input-number v-model="feedForm.feed" :min="1" :max="10" />
      </el-form-item>
    </el-form>
    <template #footer>
      <el-button @click="feedDialogVisible = false">取消</el-button>
      <el-button type="primary" @click="executeFeed">确认投喂</el-button>
    </template>
  </el-dialog>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import {
  VideoPause,
  VideoPlay,
  FullScreen,
  Warning,
  VideoCamera,
  Top,
  Bottom,
  Back,
  Right,
  Refresh,
  DataLine,
  TrendCharts,
  WarningFilled,
  Switch,
  Sunny,
  WindPower,
  Download,
  Upload,
  Food
} from '@element-plus/icons-vue'
import { getTankPrediction, getTankLatestData, controlHeater, controlAirPump, controlPumpIn, controlPumpOut, controlFeedServo } from '../api/index.js'
import { controlDevice, getDeviceStatus } from '../api/modules/automation.js'
import { ElMessage } from 'element-plus'

const videoContainerRef = ref(null)
const isPlaying = ref(true)
const streamError = ref(false)
const isLoading = ref(false)
const currentTime = ref('')
const streamTimestamp = ref(Date.now())

// 预测数据
const predictionData = ref(null)
const predictionLoading = ref(false)
const currentTankId = ref(1)

// 最新水质数据
const latestData = ref(null)
const latestDataLoading = ref(false)

// 鱼类数据
const fishData = ref(null)

// 设备状态
const deviceStatus = ref({
  heater: false,
  pumpIn: false,
  pumpOut: false,
  airPump: false
})
const deviceLoading = ref(false)

// 设备控制等待状态
const deviceControlPending = ref({
  heater: false,
  pumpIn: false,
  pumpOut: false,
  airPump: false
})

// 投喂控制
const feedDialogVisible = ref(false)
const feedForm = ref({
  servoId: 1,
  countdown: 0,
  feed: 3
})

// 使用服务器IP获取监控流
const streamUrl = computed(() => {
  return `http://139.9.0.201:9000/stream?t=${streamTimestamp.value}`
})

let timeInterval = null
let streamRefreshInterval = null
let predictionInterval = null
let latestDataInterval = null
let deviceInterval = null

const updateTime = () => {
  const now = new Date()
  currentTime.value = now.toLocaleString('zh-CN', { hour12: false })
}

// 获取预测数据
const fetchPredictionData = async () => {
  predictionLoading.value = true
  try {
    const res = await getTankPrediction(currentTankId.value)
    predictionData.value = res.data
  } catch (error) {
    console.error('获取预测数据失败:', error)
    ElMessage.warning('水质预测数据获取失败')
  } finally {
    predictionLoading.value = false
  }
}

// 格式化预测时间
const formatPredictionTime = (timeValue) => {
  if (!timeValue) return '-'
  
  // 如果是数组格式 [2026, 4, 19, 12, 9, 25]
  if (Array.isArray(timeValue) && timeValue.length >= 6) {
    const [year, month, day, hour, minute, second] = timeValue
    return `${year}-${String(month).padStart(2, '0')}-${String(day).padStart(2, '0')} ${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}:${String(second).padStart(2, '0')}`
  }
  
  // 如果是字符串格式 "2026-04-19T12:09:25" 或 "2026-04-19 12:09:25"
  if (typeof timeValue === 'string') {
    // 替换 T 为空格，确保格式一致
    const normalized = timeValue.replace('T', ' ')
    // 如果已经是格式化好的字符串，直接返回（去掉毫秒部分）
    const match = normalized.match(/^(\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2}):(\d{2})/)
    if (match) {
      return `${match[1]}-${match[2]}-${match[3]} ${match[4]}:${match[5]}:${match[6]}`
    }
    // 尝试解析为 Date
    const date = new Date(timeValue)
    if (!isNaN(date.getTime())) {
      return `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')} ${String(date.getHours()).padStart(2, '0')}:${String(date.getMinutes()).padStart(2, '0')}:${String(date.getSeconds()).padStart(2, '0')}`
    }
  }
  
  return '-'
}

// 获取状态标签类型
const getStatusType = (status) => {
  const typeMap = {
    'normal': 'success',
    'warning': 'warning',
    'danger': 'danger'
  }
  return typeMap[status] || 'info'
}

// 获取状态文本
const getStatusText = (status) => {
  const textMap = {
    'normal': '正常',
    'warning': '警告',
    'danger': '危险'
  }
  return textMap[status] || status
}

// 获取最新水质数据
const fetchLatestData = async () => {
  latestDataLoading.value = true
  try {
    const res = await getTankLatestData(currentTankId.value)
    latestData.value = res.data
    // 同时获取鱼类数据
    await fetchFishData()
  } catch (error) {
    console.error('获取最新水质数据失败:', error)
  } finally {
    latestDataLoading.value = false
  }
}

// 获取鱼类数据
const fetchFishData = async () => {
  try {
    // 调用鱼类识别数据API
    const res = await fetch(`http://139.9.0.201:9000/api/fish-identify/latest?tankId=${currentTankId.value}`)
    if (res.ok) {
      const data = await res.json()
      if (data) {
        fishData.value = {
          bmCount: data.bmCount || 0,
          gfCount: data.gfCount || 0,
          bmDValue: data.bmDValue,
          gfDValue: data.gfDValue,
          hungerStatus: data.hungerStatus,
          dValue: data.dValue,
          recordTime: data.recordTime
        }
      }
    }
  } catch (error) {
    console.error('获取鱼类数据失败:', error)
  }
}

// 获取D值样式类
const getDValueClass = (dValue) => {
  if (dValue === null || dValue === undefined) return ''
  if (dValue < 20) return 'text-danger'  // 很饥饿
  if (dValue < 50) return 'text-warning' // 较饥饿
  return 'text-success'                  // 不饥饿
}

// 获取饥饿状态类型
const getHungerStatusType = (status) => {
  const typeMap = {
    'very_hungry': 'danger',
    'hungry': 'warning',
    'slightly_hungry': 'info',
    'not_hungry': 'success'
  }
  return typeMap[status] || 'info'
}

// 获取饥饿状态文本
const getHungerStatusText = (status) => {
  const textMap = {
    'very_hungry': '非常饥饿',
    'hungry': '饥饿',
    'slightly_hungry': '轻度饥饿',
    'not_hungry': '不饥饿'
  }
  return textMap[status] || status
}

// 格式化记录时间（支持数组格式 [2026,4,16,20,15,40] 或字符串）
const formatRecordTime = (timeValue) => {
  if (!timeValue) return '-'
  // 如果是数组格式 [year, month, day, hour, minute, second]
  if (Array.isArray(timeValue) && timeValue.length >= 6) {
    const [year, month, day, hour, minute, second] = timeValue
    return `${year}-${String(month).padStart(2, '0')}-${String(day).padStart(2, '0')} ${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}:${String(second).padStart(2, '0')}`
  }
  // 如果是字符串格式，使用 Date 解析
  const date = new Date(timeValue)
  return date.toLocaleString('zh-CN', { hour12: false })
}

// 获取设备状态
const fetchDeviceStatus = async () => {
  deviceLoading.value = true
  try {
    const statusList = await getDeviceStatus(currentTankId.value)
    // 将设备状态列表转换为对象格式
    const statusObj = {
      heater: false,
      pumpIn: false,
      pumpOut: false,
      airPump: false
    }
    
    if (Array.isArray(statusList)) {
      statusList.forEach(item => {
        switch (item.deviceType) {
          case 'heater':
            statusObj.heater = item.status
            break
          case 'inflow_pump':
            statusObj.pumpIn = item.status
            break
          case 'outflow_pump':
            statusObj.pumpOut = item.status
            break
          case 'oxygen_pump':
            statusObj.airPump = item.status
            break
        }
      })
    }
    
    deviceStatus.value = statusObj
  } catch (error) {
    console.error('获取设备状态失败:', error)
  } finally {
    deviceLoading.value = false
  }
}

// 控制加热棒
const toggleHeater = async () => {
  try {
    const newStatus = !deviceStatus.value.heater
    deviceControlPending.value.heater = true
    ElMessage.info(`正在${newStatus ? '开启' : '关闭'}加热棒...`)
    
    await controlDevice('heater', newStatus)
    
    // 等待3秒后刷新设备状态
    setTimeout(async () => {
      await fetchDeviceStatus()
      deviceControlPending.value.heater = false
      if (deviceStatus.value.heater === newStatus) {
        ElMessage.success(`加热棒已${newStatus ? '开启' : '关闭'}`)
      } else {
        ElMessage.warning('加热棒状态未改变，请检查设备')
      }
    }, 3000)
  } catch (error) {
    deviceControlPending.value.heater = false
    ElMessage.error('加热棒控制失败')
  }
}

// 控制空气泵
const toggleAirPump = async () => {
  try {
    const newStatus = !deviceStatus.value.airPump
    deviceControlPending.value.airPump = true
    ElMessage.info(`正在${newStatus ? '开启' : '关闭'}空气泵...`)
    
    await controlDevice('oxygen_pump', newStatus)
    
    // 等待3秒后刷新设备状态
    setTimeout(async () => {
      await fetchDeviceStatus()
      deviceControlPending.value.airPump = false
      if (deviceStatus.value.airPump === newStatus) {
        ElMessage.success(`空气泵已${newStatus ? '开启' : '关闭'}`)
      } else {
        ElMessage.warning('空气泵状态未改变，请检查设备')
      }
    }, 3000)
  } catch (error) {
    deviceControlPending.value.airPump = false
    ElMessage.error('空气泵控制失败')
  }
}

// 控制进水泵
const togglePumpIn = async () => {
  try {
    const newStatus = !deviceStatus.value.pumpIn
    deviceControlPending.value.pumpIn = true
    ElMessage.info(`正在${newStatus ? '开启' : '关闭'}进水泵...`)
    
    await controlDevice('inflow_pump', newStatus)
    
    // 等待3秒后刷新设备状态
    setTimeout(async () => {
      await fetchDeviceStatus()
      deviceControlPending.value.pumpIn = false
      if (deviceStatus.value.pumpIn === newStatus) {
        ElMessage.success(`进水泵已${newStatus ? '开启' : '关闭'}`)
      } else {
        ElMessage.warning('进水泵状态未改变，请检查设备')
      }
    }, 3000)
  } catch (error) {
    deviceControlPending.value.pumpIn = false
    ElMessage.error('进水泵控制失败')
  }
}

// 控制出水泵
const togglePumpOut = async () => {
  try {
    const newStatus = !deviceStatus.value.pumpOut
    deviceControlPending.value.pumpOut = true
    ElMessage.info(`正在${newStatus ? '开启' : '关闭'}出水泵...`)
    
    await controlDevice('outflow_pump', newStatus)
    
    // 等待3秒后刷新设备状态
    setTimeout(async () => {
      await fetchDeviceStatus()
      deviceControlPending.value.pumpOut = false
      if (deviceStatus.value.pumpOut === newStatus) {
        ElMessage.success(`出水泵已${newStatus ? '开启' : '关闭'}`)
      } else {
        ElMessage.warning('出水泵状态未改变，请检查设备')
      }
    }, 3000)
  } catch (error) {
    deviceControlPending.value.pumpOut = false
    ElMessage.error('出水泵控制失败')
  }
}

// 打开投喂对话框
const openFeedDialog = () => {
  feedDialogVisible.value = true
}

// 执行投喂
const executeFeed = async () => {
  try {
    await controlFeedServo(feedForm.value.servoId, {
      countdown: feedForm.value.countdown,
      feed: feedForm.value.feed
    })
    ElMessage.success('投喂任务已创建')
    feedDialogVisible.value = false
  } catch (error) {
    ElMessage.error('投喂控制失败')
  }
}

let reconnectTimer = null

const togglePlay = () => {
  isPlaying.value = !isPlaying.value
  if (isPlaying.value) {
    retryStream()
  }
}

const handleStreamError = () => {
  console.error('视频流加载失败，尝试重连或检查硬件。')
  streamError.value = true
  // 自动尝试重连
  startAutoReconnect()
}

const startAutoReconnect = () => {
  // 清除之前的定时器
  if (reconnectTimer) {
    clearTimeout(reconnectTimer)
  }
  // 3秒后自动重连
  reconnectTimer = setTimeout(() => {
    if (isPlaying.value && streamError.value) {
      retryStream()
    }
  }, 3000)
}

const retryStream = () => {
  streamError.value = false
  isLoading.value = true
  streamTimestamp.value = Date.now() // 强制刷新流
  // 1秒后关闭加载状态
  setTimeout(() => {
    isLoading.value = false
  }, 1000)
}

const toggleFullScreen = () => {
  const container = videoContainerRef.value
  if (!container) return
  
  if (!document.fullscreenElement) {
    container.requestFullscreen().catch(err => {
      console.error(`尝试全屏失败: ${err.message}`)
    })
  } else {
    document.exitFullscreen()
  }
}

// 添加键盘快捷键支持
const handleKeyDown = (e) => {
  // F11键切换全屏
  if (e.key === 'F11') {
    e.preventDefault()
    toggleFullScreen()
  }
  // 空格键切换播放/暂停
  if (e.key === ' ' && e.target.tagName !== 'INPUT') {
    e.preventDefault()
    togglePlay()
  }
}

onMounted(() => {
  updateTime()
  timeInterval = setInterval(updateTime, 1000)
  // 每5秒刷新一次流，防止浏览器缓存
  streamRefreshInterval = setInterval(() => {
    if (isPlaying.value && !streamError.value) {
      streamTimestamp.value = Date.now()
    }
  }, 5000)
  // 添加键盘事件监听
  window.addEventListener('keydown', handleKeyDown)
  // 获取预测数据
  fetchPredictionData()
  // 每5秒刷新一次预测数据
  predictionInterval = setInterval(fetchPredictionData, 5000)
  // 获取最新水质数据
  fetchLatestData()
  // 每10秒刷新一次最新数据
  latestDataInterval = setInterval(fetchLatestData, 10000)
  // 获取设备状态
  fetchDeviceStatus()
  // 每10秒刷新一次设备状态
  deviceInterval = setInterval(fetchDeviceStatus, 10000)
})

onUnmounted(() => {
  if (timeInterval) clearInterval(timeInterval)
  if (reconnectTimer) clearTimeout(reconnectTimer)
  if (streamRefreshInterval) clearInterval(streamRefreshInterval)
  if (deviceInterval) clearInterval(deviceInterval)
  if (predictionInterval) clearInterval(predictionInterval)
  if (latestDataInterval) clearInterval(latestDataInterval)
  // 移除键盘事件监听
  window.removeEventListener('keydown', handleKeyDown)
})
</script>

<style scoped>
.monitoring-page {
  padding: 24px;
  background-color: #f1f5f9;
  min-height: 100vh;
}

/* 顶部标题区 */
.page-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 24px;
}
.page-title {
  margin: 0 0 8px 0;
  font-size: 24px;
  font-weight: 600;
  color: #1e293b;
}
.page-subtitle {
  margin: 0;
  font-size: 14px;
  color: #64748b;
}
.live-tag {
  font-size: 14px;
  padding: 0 16px;
  height: 32px;
  display: flex;
  align-items: center;
  gap: 8px;
  font-weight: bold;
  background-color: #10b981;
  border-color: #10b981;
}
.live-dot {
  width: 8px;
  height: 8px;
  background-color: #fff;
  border-radius: 50%;
  animation: pulse 1.5s infinite;
}
@keyframes pulse {
  0% { opacity: 1; transform: scale(1); }
  50% { opacity: 0.5; transform: scale(1.5); }
  100% { opacity: 1; transform: scale(1); }
}

/* 视频监控卡片 */
.video-card {
  border-radius: 12px;
  border: 1px solid #1e293b;
  overflow: hidden;
  background: #0f172a; /* 深色背景 */
  box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
}
.video-container {
  position: relative;
  width: 100%;
  aspect-ratio: 16 / 9;
  background-color: #0f172a;
  display: flex;
  align-items: center;
  justify-content: center;
}
.video-stream {
  width: 100%;
  height: 100%;
  object-fit: contain; /* 保证画面比例不被拉伸 */
}

/* 覆盖层：顶部时间与底部控制 */
.video-overlay {
  position: absolute;
  top: 0; left: 0; right: 0; bottom: 0;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  z-index: 10;
  pointer-events: none; /* 让鼠标事件穿透到下面的可能存在的元素 */
}
.overlay-top {
  padding: 16px 24px;
  background: linear-gradient(to bottom, rgba(0,0,0,0.8) 0%, transparent 100%);
  display: flex;
  justify-content: space-between;
  color: #fff;
  font-weight: 500;
  font-size: 14px;
}
.camera-info {
  display: flex;
  align-items: center;
}
.mr-2 {
  margin-right: 8px;
}
.overlay-bottom {
  padding: 16px 24px;
  background: linear-gradient(to top, rgba(0,0,0,0.8) 0%, transparent 100%);
  display: flex;
  justify-content: flex-end;
  pointer-events: auto; /* 控制栏恢复鼠标事件 */
}
.controls {
  display: flex;
  gap: 12px;
}

/* 状态展示 */
.video-error, .video-paused {
  color: #94a3b8;
  text-align: center;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 16px;
  z-index: 5;
}
.error-icon, .paused-icon {
  font-size: 48px;
}
.video-error p, .video-paused p {
  margin: 0;
  font-size: 16px;
}

/* 右侧信息卡片 */
.info-card {
  border-radius: 12px;
  border: 1px solid #e2e8f0;
  height: 100%;
}
.card-header {
  font-weight: 600;
  color: #1e293b;
  display: flex;
  align-items: center;
  gap: 8px;
}
.info-list {
  display: flex;
  flex-direction: column;
  gap: 16px;
}
.info-item {
  display: flex;
  justify-content: space-between;
  font-size: 14px;
  align-items: center;
}
.info-item .label {
  color: #64748b;
}
.info-item .value {
  color: #0f172a;
  font-weight: 600;
}
.truncate {
  max-width: 120px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  cursor: pointer;
}
.text-success { color: #10b981 !important; }
.text-danger { color: #ef4444 !important; }

/* 水质预测面板 */
.prediction-panel {
  margin: 16px 0;
}
.panel-header {
  display: flex;
  align-items: center;
  gap: 8px;
  font-weight: 600;
  color: #1e293b;
  margin-bottom: 12px;
}
.status-tag {
  margin-left: auto;
}
.prediction-loading {
  padding: 16px 0;
}
.prediction-content {
  display: flex;
  flex-direction: column;
  gap: 12px;
}
.prediction-time {
  font-size: 12px;
  color: #64748b;
  text-align: center;
}
.prediction-metrics {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 8px;
}
.metric-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 8px;
  background: #f8fafc;
  border-radius: 8px;
}
.metric-label {
  font-size: 12px;
  color: #64748b;
  margin-bottom: 4px;
}
.metric-value {
  font-size: 16px;
  font-weight: 600;
  color: #0f172a;
}
.metric-unit {
  font-size: 10px;
  color: #94a3b8;
}
.prediction-alerts {
  background: #fef3c7;
  border: 1px solid #f59e0b;
  border-radius: 8px;
  padding: 10px;
}
.alert-header {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 12px;
  font-weight: 600;
  color: #92400e;
  margin-bottom: 8px;
}
.alert-list {
  display: flex;
  flex-direction: column;
  gap: 4px;
}
.alert-item {
  font-size: 11px;
  color: #92400e;
  padding: 4px 8px;
  background: rgba(245, 158, 11, 0.1);
  border-radius: 4px;
  line-height: 1.4;
}
.prediction-confidence {
  text-align: center;
  font-size: 12px;
  color: #64748b;
}
.prediction-empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
  padding: 24px 0;
  color: #94a3b8;
}
.prediction-empty .el-icon {
  font-size: 32px;
}
.prediction-empty p {
  margin: 0;
  font-size: 14px;
}

/* 设备控制面板 */
.device-control-panel {
  margin: 16px 0;
}
.device-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 12px;
  margin-bottom: 16px;
}
.device-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 12px;
  background: #f8fafc;
  border-radius: 10px;
  cursor: pointer;
  transition: all 0.3s ease;
}
.device-item:hover {
  background: #e2e8f0;
  transform: translateY(-2px);
}
.device-icon {
  width: 40px;
  height: 40px;
  border-radius: 10px;
  background: #e2e8f0;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 20px;
  color: #64748b;
  transition: all 0.3s ease;
}
.device-icon.active {
  background: #10b981;
  color: #fff;
}
.device-info {
  display: flex;
  flex-direction: column;
  gap: 2px;
}
.device-name {
  font-size: 13px;
  font-weight: 600;
  color: #1e293b;
}
.device-status {
  font-size: 11px;
  color: #64748b;
}
.device-status.active {
  color: #10b981;
  font-weight: 600;
}
.feed-control {
  display: flex;
  justify-content: center;
}
.feed-btn {
  width: 100%;
}
.form-tip {
  margin-left: 8px;
  font-size: 12px;
  color: #94a3b8;
}

/* 云台控制 */
.action-panel h4 {
  margin: 0 0 16px 0;
  color: #1e293b;
  font-size: 14px;
  font-weight: 600;
  text-align: center;
}
.ptz-controls {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12px;
}
.ptz-row {
  display: flex;
  gap: 12px;
}

/* 响应式布局 */
@media (max-width: 1200px) {
  .el-row {
    flex-direction: column;
  }
  .el-col {
    width: 100% !important;
    margin-bottom: 24px;
  }
  .info-card {
    height: auto;
  }
}

/* 全屏样式 */
:fullscreen {
  background-color: #0f172a;
}

/* 控制按钮悬停效果 */
:deep(.el-button:hover) {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
}

/* 视频流加载动画 */
@keyframes spin {
  from { transform: rotate(0deg); }
  to { transform: rotate(360deg); }
}

.video-container::before {
  content: '';
  position: absolute;
  width: 40px;
  height: 40px;
  border: 4px solid rgba(255, 255, 255, 0.3);
  border-top: 4px solid #10b981;
  border-radius: 50%;
  animation: spin 1s linear infinite;
  opacity: 0;
  z-index: 20;
}

.video-container.loading::before {
  opacity: 1;
}

/* 鱼类D值显示样式 */
.fish-dvalue-section {
  margin-top: 16px;
  padding-top: 16px;
}

.fish-dvalue-header {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  font-weight: 600;
  color: #334155;
  margin-bottom: 12px;
}

.fish-dvalue-metrics {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 12px;
}

.fish-metric {
  background: #f8fafc;
  padding: 12px;
  border-radius: 8px;
  border: 1px solid #e2e8f0;
}

.fish-metric .metric-label {
  font-size: 12px;
  color: #64748b;
  margin-bottom: 4px;
}

.fish-metric .metric-value {
  font-size: 18px;
  font-weight: 600;
  color: #0f172a;
}

.fish-metric .metric-value.text-danger {
  color: #ef4444;
}

.fish-metric .metric-value.text-warning {
  color: #f59e0b;
}

.fish-metric .metric-value.text-success {
  color: #10b981;
}

.fish-metric .metric-unit {
  font-size: 12px;
  color: #94a3b8;
  margin-left: 4px;
}

.hunger-status {
  margin-top: 12px;
  text-align: center;
}

.fish-divider {
  margin: 16px 0;
}
</style>
