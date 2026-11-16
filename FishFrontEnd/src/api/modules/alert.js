import request from '../request'

// 解决告警
export const resolveAlert = (alertId) => {
  return request.put(`/alert/${alertId}/resolve`)
}

// 更新指定告警规则配置
export const updateAlertRule = (ruleId, data) => {
  return request.put(`/alert/rules/${ruleId}`, data)
}

// 删除指定的告警规则
export const deleteAlertRule = (ruleId) => {
  return request.delete(`/alert/rules/${ruleId}`)
}

// 获取告警规则列表
export const getAlertRules = (params) => {
  return request.get('/alert/rules', { params })
}

// 创建告警规则
export const createAlertRule = (data) => {
  return request.post('/alert/rules', data)
}

// 初始化默认规则
export const initAlertRules = (tankId) => {
  return request.post(`/alert/rules/init/${tankId}`)
}

// 获取未解决告警
export const getUnresolvedAlerts = (params) => {
  return request.get('/alert/unresolved', { params })
}

// 告警统计
export const getAlertStatistics = (params) => {
  return request.get('/alert/statistics', { params })
}
