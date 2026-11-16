import axios from 'axios'

// 创建独立的 axios 实例用于养鱼建议接口（不使用拦截器）
const adviceRequest = axios.create({
  timeout: 120000, // 增加到120秒，因为AI生成建议需要较长时间
  headers: {
    'Content-Type': 'application/json'
  }
})

/**
 * 获取养鱼建议
 * 后端已实现缓存机制：30分钟内相同请求会返回缓存内容
 * @param {number} tankId - 鱼缸ID
 * @param {boolean} forceRefresh - 是否强制刷新缓存
 * @returns {Promise<Object>} 养鱼建议数据（包含data, isCached, duration）
 */
export const getFishAdvice = async (tankId = 1, forceRefresh = false) => {
  try {
    const startTime = Date.now()
    const url = `http://139.9.0.201:9000/api/fish-advice/${tankId}${forceRefresh ? '?forceRefresh=true' : ''}`
    const response = await adviceRequest.get(url, {
      responseType: 'text',
      transformResponse: [(data) => {
        // 直接返回原始数据，不进行JSON解析
        return data
      }]
    })
    const endTime = Date.now()
    const duration = endTime - startTime

    // 判断是否为缓存响应（缓存响应通常很快，< 500ms）
    const isCached = !forceRefresh && duration < 500

    return {
      data: response.data,
      isCached: isCached,
      duration: duration
    }
  } catch (error) {
    console.error('获取养鱼建议API错误:', error)
    throw error
  }
}

/**
 * 批量获取多个鱼缸的养鱼建议
 * @param {number[]} tankIds - 鱼缸ID数组
 * @returns {Promise<Object>} 各鱼缸养鱼建议
 */
export const getBatchFishAdvice = async (tankIds = [1]) => {
  const results = {}
  for (const tankId of tankIds) {
    try {
      const result = await getFishAdvice(tankId)
      results[tankId] = result
    } catch (error) {
      results[tankId] = { error: error.message, isCached: false }
    }
  }
  return results
}
