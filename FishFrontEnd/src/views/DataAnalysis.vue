<template>
  <div class="automation-page">
    <!-- 顶部标题栏 -->
    <div class="page-header">
      <div class="header-left">
        <h2 class="page-title">自动化控制中心</h2>
        <p class="page-subtitle">设备联动与环境参数智能检测系统</p>
      </div>
      <div class="header-actions">
        <el-form inline size="large" class="tank-selector">
          <el-form-item label="当前鱼缸">
            <el-select v-model="currentTankId" style="width: 120px">
              <el-option label="1号缸" :value="1" />
              <el-option label="2号缸" :value="2" />
              <el-option label="3号缸" :value="3" />
            </el-select>
          </el-form-item>
        </el-form>
      </div>
    </div>

    <!-- 中间内容区 -->
    <el-row :gutter="24" class="content-area">
      <!-- 左侧：设备手动控制 -->
      <el-col :span="8">
        <el-card class="control-card" shadow="hover">
          <template #header>
            <div class="card-header">
              <div class="header-title">
                <el-icon><Operation /></el-icon>
                <span>设备手动控制</span>
              </div>
            </div>
          </template>
          <div class="device-list">
            <div v-for="device in devices" :key="device.type" class="device-item" :class="{ 'device-pending': device.loading }">
              <div class="device-info">
                <span class="device-name">{{ device.name }}</span>
                <el-tag v-if="device.loading" size="small" type="warning" class="device-status-tag">
                  <el-icon class="is-loading"><Loading /></el-icon>
                  等待响应
                </el-tag>
              </div>
              <el-switch
                v-model="device.status"
                :loading="device.loading"
                :disabled="device.loading"
                @change="handleDeviceChange(device)"
                style="--el-switch-on-color: #10b981"
              />
            </div>
          </div>

          <!-- 投喂配置 -->
          <el-divider />
          <div class="feed-config-section" v-loading="feedConfigLoading">
            <div class="section-title">
              <el-icon><Food /></el-icon>
              <span>投喂配置</span>
              <el-button type="primary" link @click="fetchFeedConfig" :loading="feedConfigLoading" size="small">
                <el-icon><RefreshRight /></el-icon>
              </el-button>
            </div>

            <!-- 全局配置 -->
            <div class="feed-config-form">
              <div class="config-item">
                <div class="config-label">
                  <span>自动投喂</span>
                </div>
                <el-switch
                  v-model="feedConfig.autoFeedEnabled"
                  :loading="autoFeedLoading"
                  @change="handleAutoFeedChange"
                  style="--el-switch-on-color: #10b981"
                />
              </div>
            </div>

            <!-- 舵机1投喂配置 -->
            <div class="fish-feed-config">
              <div class="fish-config-title">
                <span>舵机1</span>
              </div>
              <div class="feed-config-form">
                <div class="config-item">
                  <div class="config-label">
                    <span>自动投喂</span>
                  </div>
                  <el-switch
                    v-model="feedConfig.servo1AutoFeedEnabled"
                    :loading="servo1AutoFeedLoading"
                    @change="handleServo1AutoFeedChange"
                    style="--el-switch-on-color: #10b981"
                  />
                </div>
                <div class="config-item">
                  <div class="config-label">
                    <span>饥饿D值</span>
                  </div>
                  <el-input-number
                    v-model="feedConfig.servo1HungerDValue"
                    :min="0"
                    :max="200"
                    :step="5"
                    size="small"
                    style="width: 90px"
                    @change="handleServo1HungerDValueChange"
                    :disabled="servo1HungerDValueLoading"
                  />
                </div>
                <div class="config-item">
                  <div class="config-label">
                    <span>投喂时长</span>
                  </div>
                  <el-input-number
                    v-model="feedConfig.servo1FeedDuration"
                    :min="0.5"
                    :max="10"
                    :step="0.5"
                    size="small"
                    style="width: 90px"
                    @change="handleServo1FeedDurationChange"
                    :disabled="servo1FeedDurationLoading"
                  />
                </div>
              </div>
            </div>

            <!-- 舵机2投喂配置 -->
            <div class="fish-feed-config">
              <div class="fish-config-title">
                <span>舵机2</span>
              </div>
              <div class="feed-config-form">
                <div class="config-item">
                  <div class="config-label">
                    <span>自动投喂</span>
                  </div>
                  <el-switch
                    v-model="feedConfig.servo2AutoFeedEnabled"
                    :loading="servo2AutoFeedLoading"
                    @change="handleServo2AutoFeedChange"
                    style="--el-switch-on-color: #10b981"
                  />
                </div>
                <div class="config-item">
                  <div class="config-label">
                    <span>饥饿D值</span>
                  </div>
                  <el-input-number
                    v-model="feedConfig.servo2HungerDValue"
                    :min="0"
                    :max="200"
                    :step="5"
                    size="small"
                    style="width: 90px"
                    @change="handleServo2HungerDValueChange"
                    :disabled="servo2HungerDValueLoading"
                  />
                </div>
                <div class="config-item">
                  <div class="config-label">
                    <span>投喂时长</span>
                  </div>
                  <el-input-number
                    v-model="feedConfig.servo2FeedDuration"
                    :min="0.5"
                    :max="10"
                    :step="0.5"
                    size="small"
                    style="width: 90px"
                    @change="handleServo2FeedDurationChange"
                    :disabled="servo2FeedDurationLoading"
                  />
                </div>
              </div>
            </div>
          </div>
        </el-card>
      </el-col>

      <!-- 右侧：智能检测与建议 -->
      <el-col :span="16">
        <el-card class="control-card" shadow="hover">
          <template #header>
            <div class="card-header">
              <div class="header-title">
                <el-icon><Cpu /></el-icon>
                <span>环境参数智能检测</span>
              </div>
              <el-button type="primary" @click="checkAll" :icon="RefreshRight" plain>
                一键执行全量检测
              </el-button>
            </div>
          </template>

          <el-table 
            :data="parameters" 
            style="width: 100%" 
            :header-cell-style="{ background: '#f8fafc', color: '#475569', fontWeight: '600' }">
            
            <el-table-column prop="label" label="监测参数" width="140">
              <template #default="{ row }">
                <div class="param-label">
                  <strong>{{ row.label }}</strong>
                  <small>{{ row.unit }}</small>
                </div>
              </template>
            </el-table-column>
            
            <el-table-column label="当前数值" width="160">
              <template #default="{ row }">
                <el-input-number 
                  v-model="row.value" 
                  :step="row.step" 
                  size="small" 
                  style="width: 120px" 
                />
              </template>
            </el-table-column>

            <el-table-column label="诊断状态" width="100" align="center">
              <template #default="{ row }">
                <el-tag v-if="row.isOk === true" type="success" effect="light">适宜</el-tag>
                <el-tag v-else-if="row.isOk === false" type="danger" effect="light">异常</el-tag>
                <el-tag v-else type="info" effect="light">待检测</el-tag>
              </template>
            </el-table-column>

            <el-table-column label="专家建议 (AI Analysis)" min-width="220">
              <template #default="{ row }">
                <div v-loading="row.checking" class="suggestion-text">
                  {{ row.suggestion || '请点击右侧按钮进行检测' }}
                </div>
              </template>
            </el-table-column>

            <el-table-column label="操作" width="90" fixed="right" align="center">
              <template #default="{ row }">
                <el-button type="primary" link @click="checkParameter(row)" :loading="row.checking">
                  检测
                </el-button>
              </template>
            </el-table-column>
          </el-table>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import {
  Operation,
  Cpu,
  RefreshRight,
  VideoCameraFilled,
  Food,
  Loading
} from '@element-plus/icons-vue'
import { controlDevice, getDeviceStatus, getSuggestion, checkRange } from '../api/modules/automation'
import {
  getFeedConfig,
  setAutoFeedEnabled,
  setServo1AutoFeedEnabled,
  setServo1HungerDValue,
  setServo1FeedDuration,
  setServo2AutoFeedEnabled,
  setServo2HungerDValue,
  setServo2FeedDuration
} from '../api/modules/feedConfig'

const currentTankId = ref(1)

// ====================
// 1. 设备联动控制逻辑
// ====================
const devices = ref([
  { name: '加热器 (Heater)', type: 'heater', status: false, loading: false, pendingStatus: null },
  { name: '进水泵 (Inflow Pump)', type: 'inflow_pump', status: false, loading: false, pendingStatus: null },
  { name: '出水泵 (Outflow Pump)', type: 'outflow_pump', status: false, loading: false, pendingStatus: null },
  { name: '增氧泵 (Oxygen Pump)', type: 'oxygen_pump', status: false, loading: false, pendingStatus: null },
  { name: '过滤系统 (Filter)', type: 'filter', status: false, loading: false, pendingStatus: null }
])

// 设备状态轮询定时器
let deviceStatusPollingTimer = null

// 加载设备状态
const loadDeviceStatus = async () => {
  try {
    const statusList = await getDeviceStatus(currentTankId.value)
    if (statusList && statusList.length > 0) {
      statusList.forEach(status => {
        const device = devices.value.find(d => d.type === status.deviceType)
        if (device) {
          device.status = status.status
          // 如果实际状态与等待状态一致，清除等待状态
          if (device.pendingStatus !== null && device.status === device.pendingStatus) {
            device.pendingStatus = null
            device.loading = false
            ElMessage.success(`${device.name} 已${device.status ? '开启' : '关闭'}`)
          }
        }
      })
    }
  } catch (error) {
    console.error('加载设备状态失败:', error)
  }
}

// 启动设备状态轮询
const startDeviceStatusPolling = () => {
  // 立即执行一次
  loadDeviceStatus()
  // 每2秒轮询一次
  deviceStatusPollingTimer = setInterval(() => {
    loadDeviceStatus()
  }, 2000)
}

// 停止设备状态轮询
const stopDeviceStatusPolling = () => {
  if (deviceStatusPollingTimer) {
    clearInterval(deviceStatusPollingTimer)
    deviceStatusPollingTimer = null
  }
}

// 监听鱼缸切换
import { watch } from 'vue'
watch(currentTankId, () => {
  loadDeviceStatus()
})

const handleDeviceChange = async (device) => {
  // 设置等待状态
  device.pendingStatus = device.status
  device.loading = true
  
  try {
    // 调用 POST /api/automation/control-device 接口
    await controlDevice(device.type, device.status)
    ElMessage.info(`${device.name} 指令已发送，等待设备响应...`)
    // 启动轮询（如果未启动）
    if (!deviceStatusPollingTimer) {
      startDeviceStatusPolling()
    }
  } catch (error) {
    // 失败则回滚开关状态
    device.status = !device.status
    device.pendingStatus = null
    device.loading = false
    ElMessage.error(`${device.name} 指令发送失败`)
    console.error('设备控制失败', error)
  }
}

// ====================
// 2. 智能检测与建议逻辑
// ====================
const parameters = ref([
  { label: '水温', name: 'temperature', value: 26.5, unit: '°C', step: 0.5, checking: false, isOk: null, suggestion: '' },
  { label: 'pH值', name: 'ph', value: 7.2, unit: '', step: 0.1, checking: false, isOk: null, suggestion: '' },
  { label: '溶解氧', name: 'oxygen', value: 8.5, unit: 'mg/L', step: 0.1, checking: false, isOk: null, suggestion: '' }
])

const checkParameter = async (param) => {
  param.checking = true
  try {
    const payload = { 
      parameterName: param.name, 
      value: param.value, 
      tankId: currentTankId.value 
    }
    
    // 并发调用 Range 检查与 AI 建议接口
    const [rangeRes, sugRes] = await Promise.all([
      checkRange(payload),
      getSuggestion(payload)
    ])

    // 兼容后端返回结构（可能是直接返回 bool/string，也可能包在 data 中）
    param.isOk = rangeRes.data ?? rangeRes ?? true
    param.suggestion = sugRes.data ?? sugRes ?? '暂无建议'
    
  } catch (error) {
    ElMessage.error(`${param.label} 检测失败`)
    console.error('检测接口报错', error)
  } finally {
    param.checking = false
  }
}

// 一键检测全部参数
const checkAll = () => {
  parameters.value.forEach(p => checkParameter(p))
}

// ====================
// 3. 投喂配置逻辑
// ====================
const feedConfig = ref({
  autoFeedEnabled: false,
  // 舵机1配置
  servo1AutoFeedEnabled: true,
  servo1HungerDValue: 30,
  servo1FeedDuration: 2.0,
  // 舵机2配置
  servo2AutoFeedEnabled: true,
  servo2HungerDValue: 35,
  servo2FeedDuration: 1.5
})
const feedConfigLoading = ref(false)
const autoFeedLoading = ref(false)
const servo1AutoFeedLoading = ref(false)
const servo1HungerDValueLoading = ref(false)
const servo1FeedDurationLoading = ref(false)
const servo2AutoFeedLoading = ref(false)
const servo2HungerDValueLoading = ref(false)
const servo2FeedDurationLoading = ref(false)

// 获取投喂配置
const fetchFeedConfig = async () => {
  feedConfigLoading.value = true
  try {
    const res = await getFeedConfig()
    const data = res.data || res
    if (data) {
      feedConfig.value = {
        autoFeedEnabled: data.autoFeedEnabled ?? false,
        // 舵机1配置
        servo1AutoFeedEnabled: data.servo1AutoFeedEnabled ?? true,
        servo1HungerDValue: data.servo1HungerDValue ?? 30,
        servo1FeedDuration: data.servo1FeedDuration ?? 2.0,
        // 舵机2配置
        servo2AutoFeedEnabled: data.servo2AutoFeedEnabled ?? true,
        servo2HungerDValue: data.servo2HungerDValue ?? 35,
        servo2FeedDuration: data.servo2FeedDuration ?? 1.5
      }
    }
  } catch (error) {
    console.error('获取投喂配置失败:', error)
    ElMessage.error('获取投喂配置失败')
  } finally {
    feedConfigLoading.value = false
  }
}

// 处理全局自动投喂开关
const handleAutoFeedChange = async (value) => {
  autoFeedLoading.value = true
  try {
    await setAutoFeedEnabled(value)
    ElMessage.success(`自动投喂已${value ? '开启' : '关闭'}`)
  } catch (error) {
    feedConfig.value.autoFeedEnabled = !value
    console.error('设置自动投喂失败:', error)
    ElMessage.error('设置自动投喂失败')
  } finally {
    autoFeedLoading.value = false
  }
}

// ========== 舵机1配置处理 ==========

// 处理舵机1自动投喂开关
const handleServo1AutoFeedChange = async (value) => {
  servo1AutoFeedLoading.value = true
  try {
    await setServo1AutoFeedEnabled(value)
    ElMessage.success(`舵机1自动投喂已${value ? '开启' : '关闭'}`)
  } catch (error) {
    feedConfig.value.servo1AutoFeedEnabled = !value
    console.error('设置舵机1自动投喂失败:', error)
    ElMessage.error('设置舵机1自动投喂失败')
  } finally {
    servo1AutoFeedLoading.value = false
  }
}

// 处理舵机1饥饿D值变化
const handleServo1HungerDValueChange = async (value) => {
  servo1HungerDValueLoading.value = true
  try {
    await setServo1HungerDValue(value)
    ElMessage.success(`舵机1饥饿D值阈值已设置为 ${value}`)
  } catch (error) {
    console.error('设置舵机1饥饿D值失败:', error)
    ElMessage.error('设置舵机1饥饿D值失败')
    fetchFeedConfig()
  } finally {
    servo1HungerDValueLoading.value = false
  }
}

// 处理舵机1投喂时长变化
const handleServo1FeedDurationChange = async (value) => {
  servo1FeedDurationLoading.value = true
  try {
    await setServo1FeedDuration(value)
    ElMessage.success(`舵机1投喂时长已设置为 ${value} 秒`)
  } catch (error) {
    console.error('设置舵机1投喂时长失败:', error)
    ElMessage.error('设置舵机1投喂时长失败')
    fetchFeedConfig()
  } finally {
    servo1FeedDurationLoading.value = false
  }
}

// ========== 舵机2配置处理 ==========

// 处理舵机2自动投喂开关
const handleServo2AutoFeedChange = async (value) => {
  servo2AutoFeedLoading.value = true
  try {
    await setServo2AutoFeedEnabled(value)
    ElMessage.success(`舵机2自动投喂已${value ? '开启' : '关闭'}`)
  } catch (error) {
    feedConfig.value.servo2AutoFeedEnabled = !value
    console.error('设置舵机2自动投喂失败:', error)
    ElMessage.error('设置舵机2自动投喂失败')
  } finally {
    servo2AutoFeedLoading.value = false
  }
}

// 处理舵机2饥饿D值变化
const handleServo2HungerDValueChange = async (value) => {
  servo2HungerDValueLoading.value = true
  try {
    await setServo2HungerDValue(value)
    ElMessage.success(`舵机2饥饿D值阈值已设置为 ${value}`)
  } catch (error) {
    console.error('设置舵机2饥饿D值失败:', error)
    ElMessage.error('设置舵机2饥饿D值失败')
    fetchFeedConfig()
  } finally {
    servo2HungerDValueLoading.value = false
  }
}

// 处理舵机2投喂时长变化
const handleServo2FeedDurationChange = async (value) => {
  servo2FeedDurationLoading.value = true
  try {
    await setServo2FeedDuration(value)
    ElMessage.success(`舵机2投喂时长已设置为 ${value} 秒`)
  } catch (error) {
    console.error('设置舵机2投喂时长失败:', error)
    ElMessage.error('设置舵机2投喂时长失败')
    fetchFeedConfig()
  } finally {
    servo2FeedDurationLoading.value = false
  }
}

// 初始化时获取投喂配置和设备状态
fetchFeedConfig()
onMounted(() => {
  // 启动设备状态轮询
  startDeviceStatusPolling()
})

onUnmounted(() => {
  // 组件卸载时停止轮询
  stopDeviceStatusPolling()
})
</script>

<style scoped>
.automation-page {
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
.tank-selector {
  margin-bottom: 0;
}

/* 内容区 */
.content-area {
  margin-bottom: 24px;
}
.control-card {
  border-radius: 12px;
  border: 1px solid #e2e8f0;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.05);
  height: 100%;
}
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
.header-title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 16px;
  font-weight: 600;
  color: #0f172a;
}

/* 设备列表 */
.device-list {
  display: flex;
  flex-direction: column;
  gap: 16px;
}
.device-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  background: #f8fafc;
  border-radius: 8px;
  border: 1px solid #f1f5f9;
  transition: all 0.2s ease;
}
.device-item:hover {
  background: #ffffff;
  border-color: #e2e8f0;
  box-shadow: 0 2px 4px rgba(0,0,0,0.02);
}
.device-info {
  display: flex;
  align-items: center;
  gap: 12px;
}
.device-name {
  font-weight: 500;
  color: #334155;
}
.device-item.device-pending {
  background: #fefce8;
  border-color: #fbbf24;
}
.device-status-tag {
  animation: pulse 1.5s ease-in-out infinite;
}
@keyframes pulse {
  0%, 100% {
    opacity: 1;
  }
  50% {
    opacity: 0.6;
  }
}

/* 表格定制 */
.param-label strong {
  display: block;
  color: #0f172a;
}
.param-label small {
  color: #64748b;
}
.suggestion-text {
  color: #475569;
  line-height: 1.5;
  font-size: 13px;
}
:deep(.el-table__row) {
  height: 70px;
}

/* 投喂配置区域 */
.feed-config-section {
  margin-top: 8px;
}
.section-title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  font-weight: 600;
  color: #0f172a;
  margin-bottom: 16px;
}

/* 投喂配置表单 */
.feed-config-form {
  display: flex;
  flex-direction: column;
  gap: 16px;
}
.config-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  background: #f8fafc;
  border-radius: 8px;
  border: 1px solid #f1f5f9;
  transition: all 0.2s ease;
}
.config-item:hover {
  background: #ffffff;
  border-color: #e2e8f0;
  box-shadow: 0 2px 4px rgba(0,0,0,0.02);
}
.config-label {
  display: flex;
  align-items: center;
  gap: 12px;
  font-weight: 500;
  color: #334155;
  font-size: 14px;
}
</style>