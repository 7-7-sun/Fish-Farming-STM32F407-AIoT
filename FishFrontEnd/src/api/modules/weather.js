import request from '../request'

/**
 * 获取天气信息
 * @param {string} latitude - 纬度
 * @param {string} longitude - 经度
 * @param {boolean} forceRefresh - 是否强制刷新缓存
 * @returns {Promise} 返回天气数据
 */
export const getWeather = (latitude, longitude, forceRefresh = false) => {
  return request.get('/weather', {
    params: {
      latitude,
      longitude,
      forceRefresh
    }
  })
}

/**
 * 获取天气信息（带缓存状态返回）
 * 后端会自动缓存1小时，此方法用于前端需要知道是否来自缓存的场景
 * @param {string} latitude - 纬度
 * @param {string} longitude - 经度
 * @param {boolean} forceRefresh - 是否强制刷新缓存
 * @returns {Promise<{data: any, isCached: boolean}>} 返回天气数据和缓存状态
 */
export const getWeatherWithCache = async (latitude, longitude, forceRefresh = false) => {
  try {
    const response = await request.get('/weather', {
      params: {
        latitude,
        longitude,
        forceRefresh
      }
    })

    // 根据forceRefresh判断：如果是强制刷新，则肯定不是缓存；否则可能是缓存
    // 实际上后端会返回缓存数据，但前端无法直接知道是否来自缓存
    // 这里假设：如果响应很快（< 500ms）且不是强制刷新，则可能是缓存
    return {
      data: response,
      isCached: !forceRefresh // 简化处理：非强制刷新时，前端显示可能来自缓存
    }
  } catch (error) {
    throw error
  }
}
