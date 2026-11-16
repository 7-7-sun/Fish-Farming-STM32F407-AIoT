/**
 * 逆地理编码：将经纬度转换为城市名称
 * 使用 OpenStreetMap Nominatim 免费 API
 * 
 * @param {number|string} lat 纬度
 * @param {number|string} lng 经度
 * @returns {Promise<string>} 城市/地区名称
 */
export const getCityNameFromCoords = async (lat, lng) => {
  try {
    const response = await fetch(
      `https://nominatim.openstreetmap.org/reverse?format=json&lat=${lat}&lon=${lng}&zoom=10&addressdetails=1`,
      {
        headers: {
          'Accept-Language': 'zh-CN,zh;q=0.9,en;q=0.8',
        }
      }
    )
    
    if (!response.ok) {
      throw new Error('Geocoding API error')
    }

    const data = await response.json()
    
    if (data && data.address) {
      // 优先取 city，其次取 town、county、state
      const city = data.address.city || 
                   data.address.town || 
                   data.address.county || 
                   data.address.state || 
                   '未知地区'
      
      // 如果有省份，可以组合一下：广东省 广州市
      const province = data.address.state || ''
      if (province && city && province !== city) {
        return `${province} ${city}`
      }
      return city
    }
    
    return `位置 (${lat}, ${lng})`
  } catch (error) {
    console.error('逆地理编码失败:', error)
    return `位置 (${lat}, ${lng})` // 失败时降级显示坐标
  }
}
