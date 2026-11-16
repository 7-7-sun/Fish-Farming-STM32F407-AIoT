import request from '../request'

// 获取投喂配置
export const getFeedConfig = () => {
  return request.get('/feed-config')
}

// 启用/禁用自动投喂（全局）
export const setAutoFeedEnabled = (enabled) => {
  return request.put(`/feed-config/auto-feed-enabled?enabled=${enabled}`)
}

// ========== 舵机1投喂配置 ==========

// 设置舵机1自动投喂开关
export const setServo1AutoFeedEnabled = (enabled) => {
  return request.put(`/feed-config/servo1/auto-feed-enabled?enabled=${enabled}`)
}

// 设置舵机1饥饿d值阈值
export const setServo1HungerDValue = (value) => {
  return request.put(`/feed-config/servo1/hunger-d-value?value=${value}`)
}

// 设置舵机1投喂时长
export const setServo1FeedDuration = (duration) => {
  return request.put(`/feed-config/servo1/feed-duration?duration=${duration}`)
}

// ========== 舵机2投喂配置 ==========

// 设置舵机2自动投喂开关
export const setServo2AutoFeedEnabled = (enabled) => {
  return request.put(`/feed-config/servo2/auto-feed-enabled?enabled=${enabled}`)
}

// 设置舵机2饥饿d值阈值
export const setServo2HungerDValue = (value) => {
  return request.put(`/feed-config/servo2/hunger-d-value?value=${value}`)
}

// 设置舵机2投喂时长
export const setServo2FeedDuration = (duration) => {
  return request.put(`/feed-config/servo2/feed-duration?duration=${duration}`)
}

// ========== 兼容旧API（保留向后兼容） ==========

// 设置饥饿d值阈值（默认舵机1）
export const setHungerDValue = (value) => {
  return request.put(`/feed-config/hunger-d-value?value=${value}`)
}

// 设置投喂时长（默认舵机1）
export const setFeedDuration = (duration) => {
  return request.put(`/feed-config/feed-duration?duration=${duration}`)
}

// 设置黑鱼饥饿d值阈值（兼容旧版，映射到舵机1）
export const setBmHungerDValue = (value) => {
  return request.put(`/feed-config/servo1/hunger-d-value?value=${value}`)
}

// 设置黑鱼投喂时长（兼容旧版，映射到舵机1）
export const setBmFeedDuration = (duration) => {
  return request.put(`/feed-config/servo1/feed-duration?duration=${duration}`)
}

// 设置金鱼饥饿d值阈值（兼容旧版，映射到舵机2）
export const setGfHungerDValue = (value) => {
  return request.put(`/feed-config/servo2/hunger-d-value?value=${value}`)
}

// 设置金鱼投喂时长（兼容旧版，映射到舵机2）
export const setGfFeedDuration = (duration) => {
  return request.put(`/feed-config/servo2/feed-duration?duration=${duration}`)
}
