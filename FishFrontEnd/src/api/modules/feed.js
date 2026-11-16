import request from '../request'

// 获取今日投喂统计
export const getTodayStatistics = () => {
  return request.get('/feed-records/today/statistics')
}

// 获取指定舵机投喂记录
export const getServoRecords = (servoId, limit = 20) => {
  return request.get(`/feed-records/servo/${servoId}`, { params: { limit } })
}

// 获取最近投喂记录
export const getRecentRecords = (limit = 20) => {
  return request.get('/feed-records/recent', { params: { limit } })
}

/**
 * AI 智能辅助投喂分析
 * 后端已实现缓存机制：15分钟内相同参数的请求会返回缓存内容
 * @param {Object} data - 投喂决策请求参数
 * @returns {Promise<Object>} 包含数据和缓存状态的结果
 */
export const analyzeFeedDecision = async (data) => {
  const startTime = Date.now()
  const response = await request.post('/feed-decision/analyze', data, { timeout: 60000 })
  const endTime = Date.now()
  const duration = endTime - startTime

  // 判断是否为缓存响应（缓存响应通常很快，< 500ms）
  const isCached = duration < 500

  return {
    data: response,
    isCached: isCached,
    duration: duration
  }
}

/**
 * 按重量投喂（自动计算投喂时间）
 * @param {number} servoId - 舵机ID (1或2)
 * @param {Object} params - 投喂参数
 * @param {number} params.weight - 投喂重量（克）
 * @param {number} params.countdown - 倒计时时间（分钟，可选）
 * @param {boolean} params.immediate - 是否立即执行（可选）
 * @returns {Promise<string>} 投喂结果
 */
export const feedByWeight = (servoId, params) => {
  return request.post(`/device/feed/${servoId}/by-weight`, params)
}

/**
 * 获取投喂计算器参数
 * @returns {Promise<Object>} 投喂计算参数
 */
export const getFeedCalculatorParams = () => {
  return request.get('/device/feed/calculator-params')
}

/**
 * 计算投喂时间
 * @param {number} weight - 饲料重量（克）
 * @returns {Promise<Object>} 计算结果
 */
export const calculateFeedTime = (weight) => {
  return request.get('/device/feed/calculate-time', { params: { weight } })
}

/**
 * 计算饲料重量
 * @param {number} time - 投喂时间（秒）
 * @returns {Promise<Object>} 计算结果
 */
export const calculateFeedWeight = (time) => {
  return request.get('/device/feed/calculate-weight', { params: { time } })
}
