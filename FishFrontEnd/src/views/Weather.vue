<template>
  <div class="weather-page">
    <div class="weather-header">
      <h2>实时天气 {{ currentCityName ? `· ${currentCityName}` : '' }}</h2>
      <div class="location-bar">
        <!-- 城市选择 -->
        <el-select-v2
          v-model="selectedCity"
          :options="cityOptions"
          placeholder="选择城市"
          clearable
          filterable
          :filter-method="filterCities"
          class="city-select"
          @change="handleCityChange"
        >
          <template #default="{ item }">
            <div class="city-option">
              <span class="city-name">{{ item.raw.name }}</span>
              <span class="city-province">{{ item.raw.province }}</span>
            </div>
          </template>
        </el-select-v2>

        <el-input
          v-model="lngInput"
          placeholder="请输入经度"
          class="coord-input"
        >
          <template #prepend>经度</template>
        </el-input>
        <el-input
          v-model="latInput"
          placeholder="请输入纬度"
          class="coord-input"
        >
          <template #prepend>纬度</template>
        </el-input>

        <el-button type="primary" @click="fetchWeather" :loading="loading" class="action-btn">
          <el-icon><Search /></el-icon> 查询
        </el-button>
        <el-button @click="useCurrentLocation" :loading="locating" class="action-btn is-plain">
          <el-icon><Location /></el-icon> 定位
        </el-button>
      </div>
    </div>

    <div v-if="loading" class="loading-state">
      <el-icon class="loading-icon is-loading"><Loading /></el-icon>
      <span>正在获取天气数据...</span>
    </div>

    <div v-else-if="weatherData" class="weather-content">
      <!-- 顶部卡片：当前天气 -->
      <div class="current-overview">
        <div class="current-card main-temp">
          <div class="temp-display">
            <span class="temp-value">{{ weatherData.temperature }}</span>
            <span class="temp-unit">°C</span>
          </div>
          <div class="temp-label">当前温度</div>
        </div>

        <div class="current-card">
          <div class="card-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" width="32" height="32">
              <path d="M9.59 4.59A2 2 0 1 1 11 8H2m10.59 11.41A2 2 0 1 0 14 16H2m15.73-8.27A2.5 2.5 0 1 1 19.5 12H2"/>
            </svg>
          </div>
          <div class="card-info">
            <div class="card-value">{{ weatherData.windSpeed }} m/s</div>
            <div class="card-label">风速</div>
          </div>
        </div>

        <div class="current-card">
          <div class="card-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" width="32" height="32">
              <circle cx="12" cy="12" r="3"/>
              <path d="M12 2v2m0 16v2M4.93 4.93l1.41 1.41m11.32 11.32l1.41 1.41M2 12h2m16 0h2M4.93 19.07l1.41-1.41m11.32-11.32l1.41-1.41"/>
            </svg>
          </div>
          <div class="card-info">
            <div class="card-value">{{ weatherData.windDirection }}</div>
            <div class="card-label">风向</div>
          </div>
        </div>

        <div class="current-card">
          <div class="card-icon">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" width="32" height="32">
              <path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/>
            </svg>
          </div>
          <div class="card-info">
            <div class="card-value">{{ todayPrecipitation }} mm</div>
            <div class="card-label">今日降水量</div>
          </div>
        </div>
      </div>

      <!-- 图表区域 -->
      <div class="chart-section">
        <div class="chart-header">
          <h3>七天温度趋势与降水分布</h3>
        </div>
        <div class="chart-container" ref="chartRef"></div>
      </div>

      <!-- 数据表格区域 -->
      <div class="forecast-section">
        <h3>未来天气详情</h3>
        <div class="forecast-table-wrapper">
          <el-table :data="forecastList" stripe class="forecast-table">
            <el-table-column prop="day" label="日期" min-width="120" />
            <el-table-column label="天气" min-width="140">
              <template #default="{ row }">
                <el-tag :type="getWeatherTagType(row.description)" effect="light">
                  {{ row.description }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column label="最高温度" min-width="100">
              <template #default="{ row }">
                <span class="temp-high">{{ row.maxTemp }}°C</span>
              </template>
            </el-table-column>
            <el-table-column label="最低温度" min-width="100">
              <template #default="{ row }">
                <span class="temp-low">{{ row.minTemp }}°C</span>
              </template>
            </el-table-column>
            <el-table-column label="降水量" min-width="100">
              <template #default="{ row }">
                <span>{{ row.precipitation }} mm</span>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </div>
    </div>

    <div v-else class="empty-state">
      <el-icon class="empty-icon"><Cloudy /></el-icon>
      <p>输入经纬度或点击定位按钮查询天气</p>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, nextTick, watch, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import { Location, Loading, Cloudy, Search } from '@element-plus/icons-vue'
import { getWeather } from '../api/modules/weather'
import { getTankById, updateTankLocation } from '../api/modules/tank'
import { getAllCities, searchCities } from '../api/modules/city'
import { getCityNameFromCoords } from '../utils/geo'
import * as echarts from 'echarts'

const latInput = ref('39.9042')
const lngInput = ref('116.4074')
const DEFAULT_TANK_ID = 1
const loading = ref(false)
const locating = ref(false)
const weatherData = ref(null)
const currentCityName = ref('')

// 城市选择相关
const selectedCity = ref(null)
const allCities = ref([])
const cityOptions = ref([])

const chartRef = ref(null)
let chartInstance = null

const todayPrecipitation = computed(() => {
  if (!weatherData.value?.precipitation?.length) return '--'
  return weatherData.value.precipitation[0]
})

const forecastList = computed(() => {
  if (!weatherData.value?.days) return []
  return weatherData.value.days.map((day, i) => ({
    day,
    description: weatherData.value.weatherDescription?.[i] ?? '--',
    maxTemp: weatherData.value.maxTemps?.[i] ?? '--',
    minTemp: weatherData.value.minTemps?.[i] ?? '--',
    precipitation: weatherData.value.precipitation?.[i] ?? '--',
  }))
})

const getWeatherTagType = (desc) => {
  if (!desc) return 'info'
  if (desc.includes('晴')) return 'warning'
  if (desc.includes('云') || desc.includes('阴')) return 'info'
  if (desc.includes('雨')) return 'primary'
  if (desc.includes('雪')) return 'danger'
  return 'info'
}

const initChart = () => {
  if (!chartRef.value || !weatherData.value) return

  // 每次查询因为 v-if 都会重新创建 DOM，所以我们需要重新 init
  if (chartInstance) {
    chartInstance.dispose()
  }
  chartInstance = echarts.init(chartRef.value)

  const { days, maxTemps, minTemps, precipitation } = weatherData.value

  const option = {
    tooltip: {
      trigger: 'axis',
      axisPointer: {
        type: 'cross',
        crossStyle: {
          color: '#999'
        }
      }
    },
    legend: {
      data: ['降水量', '最高温度', '最低温度'],
      bottom: 0,
      textStyle: { color: '#475569' }
    },
    grid: {
      top: '15%',
      left: '3%',
      right: '3%',
      bottom: '10%',
      containLabel: true
    },
    xAxis: [
      {
        type: 'category',
        data: days,
        axisPointer: {
          type: 'shadow'
        },
        axisLine: { lineStyle: { color: '#e2ecf0' } },
        axisLabel: { color: '#64748b' }
      }
    ],
    yAxis: [
      {
        type: 'value',
        name: '温度 (°C)',
        nameTextStyle: { color: '#64748b', padding: [0, 0, 0, 20] },
        axisLabel: { formatter: '{value} °C', color: '#64748b' },
        splitLine: { lineStyle: { color: '#e2ecf0', type: 'dashed' } }
      },
      {
        type: 'value',
        name: '降水 (mm)',
        nameTextStyle: { color: '#64748b', padding: [0, 20, 0, 0] },
        axisLabel: { formatter: '{value} mm', color: '#64748b' },
        splitLine: { show: false }
      }
    ],
    series: [
      {
        name: '降水量',
        type: 'bar',
        yAxisIndex: 1,
        itemStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: '#38bdf8' },
            { offset: 1, color: '#0ea5e9' }
          ]),
          borderRadius: [4, 4, 0, 0]
        },
        barWidth: '30%',
        data: precipitation
      },
      {
        name: '最高温度',
        type: 'line',
        yAxisIndex: 0,
        itemStyle: { color: '#ef4444' },
        lineStyle: { width: 3 },
        symbol: 'circle',
        symbolSize: 8,
        data: maxTemps
      },
      {
        name: '最低温度',
        type: 'line',
        yAxisIndex: 0,
        itemStyle: { color: '#3b82f6' },
        lineStyle: { width: 3 },
        symbol: 'circle',
        symbolSize: 8,
        data: minTemps
      }
    ]
  }

  chartInstance.setOption(option)
}

const fetchWeather = async () => {
  const lat = parseFloat(latInput.value)
  const lng = parseFloat(lngInput.value)
  if (isNaN(lat) || isNaN(lng)) {
    ElMessage.warning('请输入有效的经纬度')
    return
  }
  loading.value = true
  try {
    const res = await getWeather(latInput.value, lngInput.value)
    weatherData.value = res
    // 从天气数据中获取城市名称
    if (res.cityName) {
      currentCityName.value = res.cityName
    } else {
      // 如果后端没有返回城市名，使用地理工具获取
      const city = await getCityNameFromCoords(latInput.value, lngInput.value)
      currentCityName.value = city
    }
  } catch (e) {
    // 错误由拦截器处理
  } finally {
    loading.value = false
    // 必须在 loading 变为 false 且 DOM 重新挂载后，再初始化图表
    nextTick(() => {
      initChart()
    })
  }
}

const useCurrentLocation = async () => {
  if (!navigator.geolocation) {
    ElMessage.error('浏览器不支持地理定位')
    return
  }
  locating.value = true
  navigator.geolocation.getCurrentPosition(
    async (pos) => {
      const lat = pos.coords.latitude.toFixed(4)
      const lng = pos.coords.longitude.toFixed(4)
      latInput.value = lat
      lngInput.value = lng
      locating.value = false

      // 获取城市名称并保存到数据库
      try {
        const city = await getCityNameFromCoords(lat, lng)
        await saveLocationToTank(city, lat, lng)
        ElMessage.success(`已定位到：${city}，位置信息已保存`)
      } catch (e) {
        // 城市获取失败不影响天气查询
      }

      fetchWeather()
    },
    () => {
      ElMessage.error('获取位置失败，请手动输入经纬度')
      locating.value = false
    }
  )
}

/**
 * 保存位置信息到鱼缸
 */
const saveLocationToTank = async (city, latitude, longitude) => {
  try {
    await updateTankLocation(DEFAULT_TANK_ID, {
      city: city || '未知城市',
      latitude: parseFloat(latitude),
      longitude: parseFloat(longitude)
    })
    currentCityName.value = city
  } catch (error) {
    console.error('保存位置信息失败:', error)
    ElMessage.warning('位置获取成功，但保存到数据库失败')
  }
}

/**
 * 从数据库加载默认位置
 */
const loadDefaultLocation = async () => {
  try {
    const tank = await getTankById(DEFAULT_TANK_ID)
    if (tank && tank.latitude && tank.longitude) {
      latInput.value = tank.latitude.toString()
      lngInput.value = tank.longitude.toString()
      
      // 如果数据库中有城市名称，直接使用；否则根据经纬度获取城市
      if (tank.city && tank.city !== '未知城市') {
        currentCityName.value = tank.city
        ElMessage.success(`已加载鱼缸保存的位置：${tank.city}`)
      } else {
        // 根据经纬度获取城市名称
        try {
          const city = await getCityNameFromCoords(tank.latitude, tank.longitude)
          currentCityName.value = city
          // 更新数据库中的城市信息
          await updateTankLocation(DEFAULT_TANK_ID, {
            city: city,
            latitude: tank.latitude,
            longitude: tank.longitude
          })
          ElMessage.success(`已加载鱼缸位置并识别城市：${city}`)
        } catch (e) {
          currentCityName.value = ''
          ElMessage.success('已加载鱼缸保存的位置')
        }
      }
      
      // 自动获取天气
      fetchWeather()
    } else {
      ElMessage.info('鱼缸未设置位置，请使用定位功能、城市选择或手动输入')
    }
  } catch (error) {
    console.error('加载默认位置失败:', error)
    ElMessage.warning('加载保存的位置失败，使用默认位置')
  }
}

/**
 * 加载城市列表
 */
const loadCities = async () => {
  try {
    const cities = await getAllCities()
    allCities.value = cities
    cityOptions.value = cities.map(city => ({
      value: city.code,
      label: city.fullName,
      raw: city
    }))
  } catch (error) {
    console.error('加载城市列表失败:', error)
  }
}

/**
 * 城市搜索过滤
 */
const filterCities = async (keyword) => {
  if (!keyword) {
    cityOptions.value = allCities.value.map(city => ({
      value: city.code,
      label: city.fullName,
      raw: city
    }))
    return
  }
  try {
    const cities = await searchCities(keyword)
    cityOptions.value = cities.map(city => ({
      value: city.code,
      label: city.fullName,
      raw: city
    }))
  } catch (error) {
    console.error('搜索城市失败:', error)
  }
}

/**
 * 处理城市选择变化
 */
const handleCityChange = async (cityCode) => {
  if (!cityCode) return

  const selected = allCities.value.find(c => c.code === cityCode)
  if (selected) {
    latInput.value = selected.latitude.toString()
    lngInput.value = selected.longitude.toString()
    currentCityName.value = selected.fullName

    // 保存到数据库
    try {
      await updateTankLocation(DEFAULT_TANK_ID, {
        city: selected.fullName,
        latitude: parseFloat(selected.latitude),
        longitude: parseFloat(selected.longitude)
      })
      ElMessage.success(`已选择：${selected.fullName}，位置已保存`)
    } catch (error) {
      console.error('保存位置失败:', error)
      ElMessage.warning('城市选择成功，但保存到数据库失败')
    }

    // 自动获取天气
    fetchWeather()
  }
}

// 监听窗口大小变化以重绘图表
const handleResize = () => {
  if (chartInstance) {
    chartInstance.resize()
  }
}

onMounted(() => {
  window.addEventListener('resize', handleResize)
  // 页面加载时加载城市列表和默认位置
  loadCities()
  loadDefaultLocation()
})

onUnmounted(() => {
  window.removeEventListener('resize', handleResize)
  if (chartInstance) {
    chartInstance.dispose()
  }
})
</script>

<style scoped>
.weather-page {
  padding: 24px;
  min-height: 100%;
  background-color: var(--surface-bg);
}

.weather-header {
  margin-bottom: 28px;
  background: var(--surface-card);
  padding: 24px;
  border-radius: var(--radius-md);
  box-shadow: var(--shadow-sm);
  border: 1px solid var(--border-light);
}

.weather-header h2 {
  font-size: 20px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 20px;
}

.location-bar {
  display: flex;
  align-items: center;
  gap: 16px;
  flex-wrap: wrap;
}

.coord-input {
  width: 220px;
}

.city-select {
  width: 240px;
}

.city-option {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.city-name {
  font-weight: 500;
}

.city-province {
  font-size: 12px;
  color: var(--text-muted);
}

/* 覆盖 Element Plus 前缀样式 */
:deep(.el-input-group__prepend) {
  background-color: var(--ocean-pale);
  color: var(--text-secondary);
  font-weight: 600;
  border-color: var(--border-light);
  box-shadow: none;
  border: 1px solid var(--border-light);
  border-right: 0;
}

:deep(.el-input__wrapper) {
  box-shadow: none;
  border: 1px solid var(--border-light);
}

:deep(.el-input__wrapper:hover),
:deep(.el-input__wrapper.is-focus) {
  border-color: var(--teal-primary);
  box-shadow: 0 0 0 1px var(--teal-primary) inset;
}

.action-btn {
  padding: 8px 20px;
  height: 34px;
  font-weight: 600;
  border-radius: var(--radius-sm);
  display: flex;
  align-items: center;
  gap: 6px;
}

.action-btn.is-plain {
  background: var(--ocean-pale);
  border: 1px solid var(--border-light);
  color: var(--text-secondary);
}

.action-btn.is-plain:hover {
  background: var(--teal-subtle);
  border-color: var(--teal-primary);
  color: var(--teal-primary);
}

/* 加载状态 */
.loading-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 80px 0;
  color: var(--text-muted);
  gap: 16px;
}

.loading-icon {
  font-size: 36px;
  color: var(--teal-primary);
}

/* 当前天气概览 */
.current-overview {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 20px;
  margin-bottom: 24px;
}

.current-card {
  background: var(--surface-card);
  border-radius: var(--radius-md);
  padding: 24px;
  display: flex;
  align-items: center;
  gap: 16px;
  box-shadow: var(--shadow-sm);
  border: 1px solid var(--border-light);
  transition: box-shadow var(--transition), transform var(--transition);
}

.current-card:hover {
  box-shadow: var(--shadow-md);
  transform: translateY(-2px);
}

.current-card.main-temp {
  background: linear-gradient(135deg, var(--ocean-mid), var(--teal-primary));
  border: none;
  color: #fff;
  flex-direction: column;
  align-items: flex-start;
  gap: 8px;
}

.temp-display {
  display: flex;
  align-items: flex-start;
}

.temp-value {
  font-size: 48px;
  font-weight: 800;
  line-height: 1;
}

.temp-unit {
  font-size: 20px;
  font-weight: 600;
  margin-top: 4px;
  margin-left: 2px;
}

.temp-label {
  font-size: 14px;
  opacity: 0.85;
}

.card-icon {
  flex-shrink: 0;
  width: 48px;
  height: 48px;
  border-radius: var(--radius-sm);
  background: var(--teal-subtle);
  color: var(--teal-primary);
  display: flex;
  align-items: center;
  justify-content: center;
}

.card-info {
  flex: 1;
}

.card-value {
  font-size: 20px;
  font-weight: 700;
  color: var(--text-primary);
}

.card-label {
  font-size: 13px;
  color: var(--text-muted);
  margin-top: 2px;
}

/* 图表区域 */
.chart-section {
  background: var(--surface-card);
  border-radius: var(--radius-md);
  padding: 24px;
  box-shadow: var(--shadow-sm);
  border: 1px solid var(--border-light);
  margin-bottom: 24px;
}

.chart-header h3 {
  font-size: 16px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 16px;
}

.chart-container {
  width: 100%;
  height: 380px;
}

/* 预报表格 */
.forecast-section {
  background: var(--surface-card);
  border-radius: var(--radius-md);
  padding: 24px;
  box-shadow: var(--shadow-sm);
  border: 1px solid var(--border-light);
}

.forecast-section h3 {
  font-size: 16px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 16px;
}

.temp-high {
  color: var(--coral-red);
  font-weight: 600;
}

.temp-low {
  color: var(--teal-primary);
  font-weight: 600;
}

/* 空状态 */
.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 100px 0;
  color: var(--text-muted);
  gap: 16px;
  background: var(--surface-card);
  border-radius: var(--radius-md);
  border: 1px dashed var(--border-light);
}

.empty-icon {
  font-size: 56px;
  color: var(--border-light);
}

.empty-state p {
  font-size: 15px;
}

@media (max-width: 900px) {
  .current-overview {
    grid-template-columns: repeat(2, 1fr);
  }
}
</style>