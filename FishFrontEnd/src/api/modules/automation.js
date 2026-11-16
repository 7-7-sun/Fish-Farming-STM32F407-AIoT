import request from '../request'

// 手动控制设备 (POST, 参数放在 query 中)
export const controlDevice = (deviceType, status, tankId = 1) => {
  return request.post('/automation/control-device', null, {
    params: { deviceType, status, tankId }
  })
}

// 获取设备状态列表
export const getDeviceStatus = (tankId = 1) => {
  return request.get('/automation/device-status', {
    params: { tankId }
  })
}

// 获取单个设备状态
export const getSingleDeviceStatus = (deviceType, tankId = 1) => {
  return request.get(`/automation/device-status/${deviceType}`, {
    params: { tankId }
  })
}

// 获取控制建议
export const getSuggestion = (params) => {
  return request.get('/automation/suggestion', { params })
}

// 检查参数是否在适宜范围内
export const checkRange = (params) => {
  return request.get('/automation/check-range', { params })
}
