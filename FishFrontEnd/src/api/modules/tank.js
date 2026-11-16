import request from '../request'

/**
 * 获取所有鱼缸信息
 * @returns {Promise} 鱼缸列表
 */
export const getAllTanks = () => {
  return request.get('/tanks')
}

/**
 * 根据ID获取鱼缸信息
 * @param {number} tankId - 鱼缸ID
 * @returns {Promise} 鱼缸信息
 */
export const getTankById = (tankId) => {
  return request.get(`/tanks/${tankId}`)
}

/**
 * 更新鱼缸位置信息
 * @param {number} tankId - 鱼缸ID
 * @param {Object} location - 位置信息
 * @param {string} location.city - 城市名称
 * @param {number} location.latitude - 纬度
 * @param {number} location.longitude - 经度
 * @returns {Promise} 更新后的鱼缸信息
 */
export const updateTankLocation = (tankId, { city, latitude, longitude }) => {
  return request.put(`/tanks/${tankId}/location`, null, {
    params: {
      city,
      latitude,
      longitude
    }
  })
}

/**
 * 创建新鱼缸
 * @param {Object} tank - 鱼缸信息
 * @returns {Promise} 创建的鱼缸信息
 */
export const createTank = (tank) => {
  return request.post('/tanks', tank)
}
