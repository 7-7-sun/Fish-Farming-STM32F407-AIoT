import request from '../request'

// 创建环境参数范围
export const createEnvRange = (data) => {
  return request.post('/environment-ranges', data)
}

// 更新环境参数范围
export const updateEnvRange = (data) => {
  return request.put(`/environment-ranges/${data.id}`, data)
}

// 切换自动控制开关
export const toggleAutoControl = (id, enabled) => {
  return request.patch(`/environment-ranges/${id}/toggle-auto-control`, { enabled })
}

// 获取指定鱼缸的环境参数范围
export const getTankEnvRanges = (tankId) => {
  return request.get(`/environment-ranges/tank/${tankId}`)
}

// 获取指定鱼缸的指定参数范围
export const getTankParameterRange = (tankId, parameterName) => {
  return request.get(`/environment-ranges/tank/${tankId}/parameter/${parameterName}`)
}

// 获取默认环境参数范围
export const getDefaultEnvRanges = () => {
  return request.get('/environment-ranges/default')
}

// 删除环境参数范围
export const deleteEnvRange = (id) => {
  return request.delete(`/environment-ranges/delete/${id}`)
}
