import axios from 'axios'

// 创建独立的 axios 实例用于设备控制接口
const deviceRequest = axios.create({
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
})

// 请求拦截器 - 添加认证token
deviceRequest.interceptors.request.use(
  (config) => {
    const token = localStorage.getItem('token') || sessionStorage.getItem('token')
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  (error) => {
    return Promise.reject(error)
  }
)

// 响应拦截器 - 处理响应和错误
deviceRequest.interceptors.response.use(
  (response) => {
    return response
  },
  (error) => {
    console.error('设备控制请求失败:', error)
    if (error.response) {
      console.error('错误状态码:', error.response.status)
      console.error('错误响应:', error.response.data)
    } else if (error.request) {
      console.error('请求已发送但没有收到响应')
    } else {
      console.error('请求配置错误:', error.message)
    }
    return Promise.reject(error)
  }
)

/**
 * 控制加热棒
 * @param {boolean} status - true: 开启, false: 关闭
 * @returns {Promise} 控制结果
 */
export const controlHeater = (status) => {
  return deviceRequest.post('http://139.9.0.201:9000/api/device/heater', status)
}

/**
 * 控制空气泵
 * @param {boolean} status - true: 开启, false: 关闭
 * @returns {Promise} 控制结果
 */
export const controlAirPump = (status) => {
  return deviceRequest.post('http://139.9.0.201:9000/api/device/air-pump', status)
}

/**
 * 控制进水泵
 * @param {boolean} status - true: 开启, false: 关闭
 * @returns {Promise} 控制结果
 */
export const controlPumpIn = (status) => {
  return deviceRequest.post('http://139.9.0.201:9000/api/device/pump-in', status)
}

/**
 * 控制出水泵
 * @param {boolean} status - true: 开启, false: 关闭
 * @returns {Promise} 控制结果
 */
export const controlPumpOut = (status) => {
  return deviceRequest.post('http://139.9.0.201:9000/api/device/pump-out', status)
}

/**
 * 控制投喂舵机
 * @param {number} servoId - 舵机ID (1表示舵机1, 2表示舵机2)
 * @param {Object} params - 控制参数
 * @param {number} params.countdown - 倒计时时间(分钟)
 * @param {number} params.feed - 投喂时间(秒)
 * @returns {Promise} 控制结果
 */
export const controlFeedServo = (servoId, params) => {
  return deviceRequest.post(`http://139.9.0.201:9000/api/device/feed/${servoId}`, params)
}

/**
 * 获取设备状态（从最新水质数据中提取）
 * @param {number} tankId - 鱼缸ID
 * @returns {Promise<Object>} 设备状态
 */
export const getDeviceStatus = async (tankId = 1) => {
  try {
    const res = await deviceRequest.get(`http://139.9.0.201:9000/api/tank-data/latest?tankId=${tankId}`)
    const data = res.data
    return {
      heater: data.heaterStatus,        // 加热棒状态
      pumpIn: data.pumpInStatus,        // 进水泵状态
      pumpOut: data.pumpOutStatus,      // 出水泵状态
      airPump: data.airPumpStatus,      // 空气泵状态
      recordTime: data.recordTime       // 记录时间
    }
  } catch (error) {
    console.error('获取设备状态失败:', error)
    throw error
  }
}

/**
 * 批量控制多个设备
 * @param {Object} commands - 设备控制命令
 * @param {boolean} commands.heater - 加热棒
 * @param {boolean} commands.airPump - 空气泵
 * @param {boolean} commands.pumpIn - 进水泵
 * @param {boolean} commands.pumpOut - 出水泵
 * @returns {Promise<Object>} 各设备控制结果
 */
export const batchControlDevices = async (commands) => {
  const results = {}
  const promises = []

  if (commands.heater !== undefined) {
    promises.push(
      controlHeater(commands.heater)
        .then(() => { results.heater = { success: true } })
        .catch(err => { results.heater = { success: false, error: err.message } })
    )
  }

  if (commands.airPump !== undefined) {
    promises.push(
      controlAirPump(commands.airPump)
        .then(() => { results.airPump = { success: true } })
        .catch(err => { results.airPump = { success: false, error: err.message } })
    )
  }

  if (commands.pumpIn !== undefined) {
    promises.push(
      controlPumpIn(commands.pumpIn)
        .then(() => { results.pumpIn = { success: true } })
        .catch(err => { results.pumpIn = { success: false, error: err.message } })
    )
  }

  if (commands.pumpOut !== undefined) {
    promises.push(
      controlPumpOut(commands.pumpOut)
        .then(() => { results.pumpOut = { success: true } })
        .catch(err => { results.pumpOut = { success: false, error: err.message } })
    )
  }

  await Promise.all(promises)
  return results
}
