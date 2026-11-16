import request from '../request'

/**
 * 获取所有城市列表
 * @returns {Promise<Array>} 城市列表
 */
export const getAllCities = () => {
  return request.get('/cities')
}

/**
 * 搜索城市
 * @param {string} keyword - 搜索关键词
 * @returns {Promise<Array>} 匹配的城市列表
 */
export const searchCities = (keyword) => {
  return request.get('/cities/search', {
    params: { keyword }
  })
}

/**
 * 根据城市代码获取城市信息
 * @param {string} code - 城市行政区划代码
 * @returns {Promise<Object>} 城市信息
 */
export const getCityByCode = (code) => {
  return request.get(`/cities/${code}`)
}
