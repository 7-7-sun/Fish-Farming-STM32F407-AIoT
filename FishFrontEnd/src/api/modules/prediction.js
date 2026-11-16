import axios from 'axios'

// 创建独立的 axios 实例用于预测接口（不使用统一的 baseURL）
const predictionRequest = axios.create({
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
})

/**
 * 获取鱼缸水质预测数据
 * @param {number} tankId - 鱼缸ID
 * @returns {Promise} 预测数据
 * {
 *   predictionTime: [2026,4,16,12,53,32,285074305],
 *   predictions: {
 *     turbidity: 814.2,      // 浊度
 *     phValue: 5.38,         // pH值
 *     temperature: 24.75,    // 温度
 *     dissolvedOxygen: 7.1   // 溶解氧
 *   },
 *   alerts: {
 *     turbidity: "...",
 *     phValue: "..."
 *   },
 *   status: "warning",
 *   confidence: 0.85
 * }
 */
export const getTankPrediction = (tankId = 1) => {
  return predictionRequest.get(`http://139.9.0.201:9000/api/tank-data/predict?tankId=${tankId}`)
}

/**
 * 批量获取多个鱼缸的预测数据
 * @param {number[]} tankIds - 鱼缸ID数组
 * @returns {Promise<Object>} 各鱼缸预测数据
 */
export const getBatchPredictions = async (tankIds = [1]) => {
  const results = {}
  for (const tankId of tankIds) {
    try {
      const res = await getTankPrediction(tankId)
      results[tankId] = res.data
    } catch (error) {
      results[tankId] = { error: error.message }
    }
  }
  return results
}

/**
 * 获取鱼缸最新水质数据
 * @param {number} tankId - 鱼缸ID
 * @returns {Promise} 最新水质数据
 * {
 *   id: 1,
 *   tankId: 1,
 *   turbidity: 10.5,
 *   phValue: 7.2,
 *   temperature: 25.0,
 *   dissolvedOxygen: 6.8,
 *   recordTime: "2026-04-16T12:53:32"
 * }
 */
export const getTankLatestData = (tankId = 1) => {
  return predictionRequest.get(`http://139.9.0.201:9000/api/tank-data/latest?tankId=${tankId}`)
}

/**
 * 批量获取多个鱼缸的最新数据
 * @param {number[]} tankIds - 鱼缸ID数组
 * @returns {Promise<Object>} 各鱼缸最新数据
 */
export const getBatchLatestData = async (tankIds = [1]) => {
  const results = {}
  for (const tankId of tankIds) {
    try {
      const res = await getTankLatestData(tankId)
      results[tankId] = res.data
    } catch (error) {
      results[tankId] = { error: error.message }
    }
  }
  return results
}
