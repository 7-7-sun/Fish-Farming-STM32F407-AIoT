<template>
  <div class="feed-records-page">
    <div class="page-header">
      <h2 class="page-title">投喂记录管理</h2>
      <p class="page-subtitle">查看鱼缸的每日投喂统计与详细历史记录</p>
    </div>

    <!-- 统计卡片区域 -->
    <el-row :gutter="20" class="statistics-row" v-loading="loadingStats">
      <el-col :span="8" v-for="(count, servoId) in todayStats" :key="servoId">
        <el-card shadow="hover" class="stat-card" :class="`stat-card-${servoId}`">
          <div class="stat-content">
            <div class="stat-icon">
              <el-icon v-if="servoId === 'total'"><DataLine /></el-icon>
              <el-icon v-else><Odometer /></el-icon>
            </div>
            <div class="stat-info">
              <div class="stat-value">{{ count }} <span class="stat-unit">次</span></div>
              <div class="stat-label" v-if="servoId === 'total'">今日总投喂</div>
              <div class="stat-label" v-else>今日舵机 {{ servoId }} 投喂</div>
            </div>
          </div>
        </el-card>
      </el-col>
      <el-col :span="8" v-if="Object.keys(todayStats).length === 0 && !loadingStats">
        <el-card shadow="hover" class="stat-card empty-stat">
          <div class="stat-content">
            <div class="stat-icon"><el-icon><Warning /></el-icon></div>
            <div class="stat-info">
              <div class="stat-value">0 <span class="stat-unit">次</span></div>
              <div class="stat-label">今日暂无投喂数据</div>
            </div>
          </div>
        </el-card>
      </el-col>
    </el-row>

    <!-- 图表可视化区域 -->
    <el-row :gutter="20" class="charts-row">
      <el-col :span="8">
        <el-card shadow="never" class="chart-card">
          <template #header>
            <div class="card-header">
              <span>今日投喂分布</span>
            </div>
          </template>
          <div ref="pieChartRef" class="chart-container"></div>
        </el-card>
      </el-col>
      <el-col :span="16">
        <el-card shadow="never" class="chart-card">
          <template #header>
            <div class="card-header">
              <span>最近投喂时长趋势</span>
            </div>
          </template>
          <div ref="barChartRef" class="chart-container"></div>
        </el-card>
      </el-col>
    </el-row>

    <!-- AI 辅助投喂区域 -->
    <el-card class="ai-decision-card" shadow="never">
      <template #header>
        <div class="ai-header">
          <div>
            <h3 class="ai-title">智能 AI 辅助投喂</h3>
            <p class="ai-subtitle">结合鱼只数量、浮头次数和最近记录，给出本次投喂建议</p>
          </div>
          <el-tag type="danger" effect="dark">AI 决策</el-tag>
        </div>
      </template>

      <div class="ai-decision-layout">
        <div class="ai-form-panel">
          <div class="ai-panel-intro">
            <span class="ai-panel-badge">实时分析</span>
            <span>建议先确认鱼缸编号和鱼只数量，再结合最近观察到的浮头情况发起分析。</span>
          </div>

          <el-form :model="aiDecisionForm" label-width="150px" class="ai-form">
            <el-row :gutter="16">
              <el-col :xs="24" :md="12">
                <el-form-item label="鱼缸 ID">
                  <el-input-number v-model="aiDecisionForm.tankId" :min="1" :step="1" controls-position="right" class="full-width" />
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="浮头次数">
                  <el-input-number v-model="aiDecisionForm.surfaceApproachCount" :min="0" :step="1" controls-position="right" class="full-width" />
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="黑鱼数量">
                  <el-input-number v-model="aiDecisionForm.blackFishCount" :min="0" :step="1" controls-position="right" class="full-width" />
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="金鱼数量">
                  <el-input-number v-model="aiDecisionForm.goldFishCount" :min="0" :step="1" controls-position="right" class="full-width" />
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="黑鱼单次建议量">
                  <el-input-number
                    v-model="aiDecisionForm.recommendedFeedPerBlackFish"
                    :min="0"
                    :step="0.1"
                    :precision="1"
                    controls-position="right"
                    class="full-width" />
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="金鱼单次建议量">
                  <el-input-number
                    v-model="aiDecisionForm.recommendedFeedPerGoldFish"
                    :min="0"
                    :step="0.1"
                    :precision="1"
                    controls-position="right"
                    class="full-width" />
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="分析记录条数">
                  <el-input-number v-model="aiDecisionForm.recentRecordLimit" :min="1" :max="50" :step="1" controls-position="right" class="full-width" />
                </el-form-item>
              </el-col>
            </el-row>

            <div class="ai-action-row">
              <el-button type="primary" :loading="loadingAiDecision" @click="handleAiDecision">
                {{ loadingAiDecision ? '分析中...' : '生成 AI 投喂建议' }}
              </el-button>
              <el-button @click="applyRecentTankId">使用最近记录鱼缸</el-button>
              <el-button @click="fillAiFormDefaults">恢复默认参数</el-button>
            </div>
          </el-form>
        </div>

        <div class="ai-result-panel">
          <template v-if="aiDecisionResult">
            <div class="ai-result-top">
              <div>
                <div class="result-kicker">
                  AI 决策结果
                  <el-tag v-if="isAiDecisionFromCache" type="warning" effect="plain" size="small" style="margin-left: 8px;">
                    <el-icon><Collection /></el-icon>
                    缓存
                  </el-tag>
                </div>
                <div class="result-time">{{ formatTime(aiDecisionResult.decisionTime) }}</div>
              </div>
              <el-tag :type="aiDecisionResult.shouldFeed ? 'success' : 'warning'" size="large">
                {{ aiDecisionResult.shouldFeed ? '建议投喂' : '建议暂缓投喂' }}
              </el-tag>
            </div>

            <el-alert
              :type="aiDecisionResult.shouldFeed ? 'success' : 'warning'"
              :closable="false"
              show-icon>
              <template #title>
                <div class="md-content" v-html="parseMarkdown(aiDecisionResult.overallAdvice || '本次分析已完成')"></div>
              </template>
            </el-alert>

            <div class="recommend-grid">
              <div class="recommend-card" v-if="aiDecisionResult.blackFishRecommendation">
                <div class="recommend-title">黑鱼建议</div>
                <div class="recommend-amount">{{ formatAmount(aiDecisionResult.blackFishRecommendation.suggestedAmount) }}</div>
                <div class="recommend-meta">
                  {{ aiDecisionResult.blackFishRecommendation.currentCount }} 条 · {{ aiDecisionResult.blackFishRecommendation.recommendedFeedName || '未提供饲料名称' }}
                </div>
                <div class="recommend-scrollable">
                  <div class="md-content recommend-reason" v-html="parseMarkdown(aiDecisionResult.blackFishRecommendation.reason || '暂无详细原因说明')"></div>
                  <div class="md-content recommend-note" v-html="parseMarkdown('注意事项：' + (aiDecisionResult.blackFishRecommendation.precautions || '请根据鱼群状态灵活调整。'))"></div>
                </div>
              </div>

              <div class="recommend-card recommend-card-gold" v-if="aiDecisionResult.goldFishRecommendation">
                <div class="recommend-title">金鱼建议</div>
                <div class="recommend-amount">{{ formatAmount(aiDecisionResult.goldFishRecommendation.suggestedAmount) }}</div>
                <div class="recommend-meta">
                  {{ aiDecisionResult.goldFishRecommendation.currentCount }} 条 · {{ aiDecisionResult.goldFishRecommendation.recommendedFeedName || '未提供饲料名称' }}
                </div>
                <div class="recommend-scrollable">
                  <div class="md-content recommend-reason" v-html="parseMarkdown(aiDecisionResult.goldFishRecommendation.reason || '暂无详细原因说明')"></div>
                  <div class="md-content recommend-note" v-html="parseMarkdown('注意事项：' + (aiDecisionResult.goldFishRecommendation.precautions || '请根据鱼群状态灵活调整。'))"></div>
                </div>
              </div>
            </div>

            <div class="ai-analysis-box">
              <div class="analysis-header">
                <span>综合分析</span>
                <el-tag type="info" effect="plain">已分析 {{ aiDecisionResult.recordsAnalyzed ?? aiDecisionForm.recentRecordLimit }} 条记录</el-tag>
              </div>
              <div class="md-content analysis-body" v-html="parseMarkdown(aiDecisionResult.aiAnalysisText || '接口未返回详细分析文本。')"></div>
            </div>

            <!-- 重新生成按钮 -->
            <div class="ai-result-actions">
              <el-button 
                type="primary" 
                :icon="Refresh" 
                :loading="loadingAiDecision" 
                @click="handleRegenerateDecision"
              >
                {{ loadingAiDecision ? '分析中...' : '重新生成建议' }}
              </el-button>
              <el-tag v-if="isAiDecisionFromCache" type="info" effect="plain" size="small">
                当前为缓存结果，重新生成将获取最新分析
              </el-tag>
            </div>
          </template>

          <div v-else class="ai-result-empty">
            <el-empty description="填写参数后即可生成 AI 辅助投喂建议" :image-size="90" />
          </div>
        </div>
      </div>
    </el-card>

    <!-- 按重量投喂区域 -->
    <el-card class="weight-feed-card" shadow="never">
      <template #header>
        <div class="weight-feed-header">
          <div>
            <h3 class="weight-feed-title">
              <el-icon><ScaleToOriginal /></el-icon>
              按重量精准投喂
            </h3>
            <p class="weight-feed-subtitle">根据饲料重量自动计算投喂时间，56.5g饲料约需29秒（1.95g/秒）</p>
          </div>
          <el-tag type="success" effect="dark">精准控制</el-tag>
        </div>
      </template>

      <div class="weight-feed-layout">
        <div class="weight-feed-form-panel">
          <div class="weight-feed-intro">
            <span class="weight-feed-badge">智能计算</span>
            <span>输入饲料重量，系统自动计算所需投喂时间，支持定时投喂</span>
          </div>

          <el-form :model="weightFeedForm" label-width="120px" class="weight-feed-form">
            <el-row :gutter="16">
              <el-col :xs="24" :md="12">
                <el-form-item label="选择舵机">
                  <el-radio-group v-model="weightFeedForm.servoId" size="large">
                    <el-radio-button :label="1">舵机 1</el-radio-button>
                    <el-radio-button :label="2">舵机 2</el-radio-button>
                  </el-radio-group>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="饲料重量">
                  <el-input-number
                    v-model="weightFeedForm.weight"
                    :min="0.1"
                    :max="30"
                    :step="0.5"
                    :precision="1"
                    controls-position="right"
                    class="full-width"
                    @change="handleWeightChange">
                    <template #suffix>克</template>
                  </el-input-number>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="计算投喂时间">
                  <el-input
                    v-model="calculatedTime"
                    readonly
                    class="full-width">
                    <template #suffix>秒</template>
                  </el-input>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12">
                <el-form-item label="执行方式">
                  <el-radio-group v-model="weightFeedForm.immediate" size="large">
                    <el-radio-button :label="true">立即执行</el-radio-button>
                    <el-radio-button :label="false">定时执行</el-radio-button>
                  </el-radio-group>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :md="12" v-if="!weightFeedForm.immediate">
                <el-form-item label="倒计时">
                  <el-input-number
                    v-model="weightFeedForm.countdown"
                    :min="1"
                    :max="120"
                    :step="5"
                    controls-position="right"
                    class="full-width">
                    <template #suffix>分钟</template>
                  </el-input-number>
                </el-form-item>
              </el-col>
            </el-row>

            <div class="weight-feed-action-row">
              <el-button 
                type="success" 
                size="large"
                :icon="Food"
                :loading="loadingWeightFeed" 
                @click="handleWeightFeed">
                {{ loadingWeightFeed ? '投喂中...' : '开始投喂' }}
              </el-button>
              <el-button @click="resetWeightFeedForm">重置</el-button>
            </div>
          </el-form>

          <!-- 快速选择按钮 -->
          <div class="quick-weight-buttons">
            <span class="quick-label">快速选择：</span>
            <el-button-group>
              <el-button v-for="w in [5, 10, 15, 20, 25]" :key="w" @click="setQuickWeight(w)">
                {{ w }}g
              </el-button>
            </el-button-group>
          </div>
        </div>

        <div class="weight-feed-info-panel">
          <div class="feed-params-info">
            <h4>投喂参数</h4>
            <div class="param-item">
              <span class="param-label">投喂速度：</span>
              <span class="param-value">{{ feedParams.feedRateGramsPerSecond }} g/秒</span>
            </div>
            <div class="param-item">
              <span class="param-label">每克所需时间：</span>
              <span class="param-value">{{ feedParams.timePerGram }} 秒/g</span>
            </div>
            <div class="param-item">
              <span class="param-label">单次最大投喂：</span>
              <span class="param-value">{{ feedParams.maxFeedWeightPerTime }} g</span>
            </div>
            <div class="param-desc">{{ feedParams.description }}</div>
          </div>

          <div class="feed-calculator-demo">
            <h4>计算示例</h4>
            <div class="demo-item">
              <span class="demo-weight">10g</span>
              <span class="demo-arrow">→</span>
              <span class="demo-time">约 {{ (10 * feedParams.timePerGram).toFixed(1) }} 秒</span>
            </div>
            <div class="demo-item">
              <span class="demo-weight">20g</span>
              <span class="demo-arrow">→</span>
              <span class="demo-time">约 {{ (20 * feedParams.timePerGram).toFixed(1) }} 秒</span>
            </div>
            <div class="demo-item">
              <span class="demo-weight">30g</span>
              <span class="demo-arrow">→</span>
              <span class="demo-time">约 {{ (30 * feedParams.timePerGram).toFixed(1) }} 秒</span>
            </div>
          </div>
        </div>
      </div>
    </el-card>

    <!-- 记录详情区域 -->
    <el-card class="records-card" shadow="never">
      <div class="records-header">
        <div class="header-tabs">
          <el-radio-group v-model="currentTab" @change="handleTabChange" size="large">
            <el-radio-button value="recent">最近投喂记录</el-radio-button>
            <el-radio-button value="servo1">舵机 1 记录</el-radio-button>
            <el-radio-button value="servo2">舵机 2 记录</el-radio-button>
          </el-radio-group>
        </div>
        <div class="header-actions">
          <el-button type="primary" :icon="Refresh" circle @click="fetchRecords" :loading="loadingRecords" />
        </div>
      </div>

      <el-table
        :data="recordsList"
        style="width: 100%"
        v-loading="loadingRecords"
        stripe
        class="custom-table"
        :header-cell-style="{ background: 'var(--ocean-pale)', color: 'var(--text-primary)', fontWeight: '600' }">
        <el-table-column prop="recordId" label="记录ID" width="100" align="center" />
        <el-table-column prop="tankId" label="鱼缸ID" width="120" align="center">
          <template #default="scope">
            <el-tag size="small" type="info">Tank {{ scope.row.tankId }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="servoId" label="投喂舵机" width="150" align="center">
          <template #default="scope">
            <el-tag :type="scope.row.servoId === 1 ? 'primary' : 'success'" effect="light">
              <el-icon style="vertical-align: middle; margin-right: 4px"><Setting /></el-icon>
              舵机 {{ scope.row.servoId }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="feedDuration" label="投喂时长 (毫秒)" width="150" align="center">
          <template #default="scope">
            <span class="duration-text">{{ scope.row.feedDuration }} ms</span>
          </template>
        </el-table-column>
        <el-table-column prop="recordTime" label="投喂时间" min-width="200" align="center">
          <template #default="scope">
            <div class="time-cell">
              <el-icon><Calendar /></el-icon>
              <span>{{ formatTime(scope.row.recordTime) }}</span>
            </div>
          </template>
        </el-table-column>
        <template #empty>
          <el-empty description="暂无投喂记录" :image-size="100" />
        </template>
      </el-table>
    </el-card>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted, onUnmounted, nextTick } from 'vue'
import { ElMessage } from 'element-plus'
import { Odometer, Refresh, Setting, Calendar, Warning, DataLine, Collection, Food, ScaleToOriginal } from '@element-plus/icons-vue'
import { getTodayStatistics, getServoRecords, getRecentRecords, analyzeFeedDecision, feedByWeight, getFeedCalculatorParams } from '../api/modules/feed'
import * as echarts from 'echarts'
import { computed } from 'vue'

// 状态变量
const todayStats = ref({})
const loadingStats = ref(false)

const currentTab = ref('recent')
const recordsList = ref([])
const loadingRecords = ref(false)

const createDefaultAiDecisionForm = () => ({
  tankId: 1,
  blackFishCount: 5,
  goldFishCount: 8,
  recommendedFeedPerBlackFish: 0.5,
  recommendedFeedPerGoldFish: 0.3,
  surfaceApproachCount: 3,
  recentRecordLimit: 10
})

const aiDecisionForm = reactive(createDefaultAiDecisionForm())
const loadingAiDecision = ref(false)
const aiDecisionResult = ref(null)

// 按重量投喂相关
const weightFeedForm = reactive({
  servoId: 1,
  weight: 10,
  immediate: true,
  countdown: 5
})
const loadingWeightFeed = ref(false)
const feedParams = ref({
  feedRateGramsPerSecond: 1.948,
  timePerGram: 0.513,
  maxFeedWeightPerTime: 30,
  description: '56.5g饲料约需29秒投喂完成'
})

// 计算投喂时间
const calculatedTime = computed(() => {
  const time = weightFeedForm.weight * feedParams.value.timePerGram
  return time.toFixed(1)
})

// 图表引用
const pieChartRef = ref(null)
const barChartRef = ref(null)
let pieChart = null
let barChart = null

// 初始化饼图
const initPieChart = () => {
  if (!pieChartRef.value) return
  if (!pieChart) pieChart = echarts.init(pieChartRef.value)
  
  const data = []
  if (todayStats.value['1']) data.push({ value: todayStats.value['1'], name: '舵机 1', itemStyle: { color: '#0ea5e9' } })
  if (todayStats.value['2']) data.push({ value: todayStats.value['2'], name: '舵机 2', itemStyle: { color: '#10b981' } })

  const option = {
    tooltip: {
      trigger: 'item',
      formatter: '{b}: {c} 次 ({d}%)'
    },
    legend: {
      bottom: '5%',
      left: 'center'
    },
    series: [
      {
        name: '投喂次数',
        type: 'pie',
        radius: ['40%', '70%'],
        avoidLabelOverlap: false,
        itemStyle: {
          borderRadius: 10,
          borderColor: '#fff',
          borderWidth: 2
        },
        label: {
          show: false,
          position: 'center'
        },
        emphasis: {
          label: {
            show: true,
            fontSize: 18,
            fontWeight: 'bold'
          }
        },
        labelLine: {
          show: false
        },
        data: data.length > 0 ? data : [{ value: 0, name: '暂无数据', itemStyle: { color: '#e5e7eb' } }]
      }
    ]
  }
  
  pieChart.setOption(option)
}

// 初始化柱状图
const initBarChart = () => {
  if (!barChartRef.value) return
  if (!barChart) barChart = echarts.init(barChartRef.value)

  // 取最近10条数据，反转顺序以便时间正序显示
  const chartData = [...recordsList.value].slice(0, 10).reverse()
  
  const xAxisData = chartData.map(item => {
    const d = parseDate(item.recordTime)
    return `${d.getHours().toString().padStart(2, '0')}:${d.getMinutes().toString().padStart(2, '0')}`
  })
  
  const seriesData1 = chartData.map(item => item.servoId === 1 ? item.feedDuration : 0)
  const seriesData2 = chartData.map(item => item.servoId === 2 ? item.feedDuration : 0)

  const option = {
    tooltip: {
      trigger: 'axis',
      axisPointer: { type: 'shadow' }
    },
    legend: {
      data: ['舵机 1', '舵机 2'],
      top: 0
    },
    grid: {
      left: '3%',
      right: '4%',
      bottom: '3%',
      containLabel: true
    },
    xAxis: {
      type: 'category',
      data: xAxisData.length > 0 ? xAxisData : ['-'],
      axisLine: { lineStyle: { color: '#9ca3af' } }
    },
    yAxis: {
      type: 'value',
      name: '时长 (ms)',
      axisLine: { show: false },
      splitLine: { lineStyle: { type: 'dashed', color: '#e5e7eb' } }
    },
    series: [
      {
        name: '舵机 1',
        type: 'bar',
        stack: 'total',
        itemStyle: { color: '#0ea5e9', borderRadius: [4, 4, 0, 0] },
        barWidth: '30%',
        data: seriesData1
      },
      {
        name: '舵机 2',
        type: 'bar',
        stack: 'total',
        itemStyle: { color: '#10b981', borderRadius: [4, 4, 0, 0] },
        barWidth: '30%',
        data: seriesData2
      }
    ]
  }

  barChart.setOption(option)
}

// 监听窗口大小改变，重绘图表
const handleResize = () => {
  if (pieChart) pieChart.resize()
  if (barChart) barChart.resize()
}

// 统一的日期解析函数，兼容字符串和数组格式
const parseDate = (timeData) => {
  if (!timeData) return new Date()
  // 如果是数组格式，例如 [2024, 4, 12, 11, 25, 19, 168]
  if (Array.isArray(timeData)) {
    const [year, month, day, hour = 0, minute = 0, second = 0] = timeData
    return new Date(year, month - 1, day, hour, minute, second)
  }
  // 如果是字符串或时间戳
  const date = new Date(timeData)
  return isNaN(date.getTime()) ? new Date() : date
}

// 格式化时间
const formatTime = (timeStr) => {
  if (!timeStr) return '-'
  const date = parseDate(timeStr)
  return date.toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  }).replace(/\//g, '-')
}

const formatAmount = (amount) => {
  if (amount === undefined || amount === null || amount === '') return '-'
  return Number.isFinite(Number(amount)) ? `${Number(amount)} 份` : String(amount)
}

// 简易 Markdown 解析器
const parseMarkdown = (text) => {
  if (!text) return ''
  let html = text
    // 清理后端返回文本里常见的冗余前缀
    .replace(/^注意事项[:：]?\s*(注意事项[:：]?)?\s*\*/, '*')
    // 替换加粗 **text** -> <strong>text</strong>，支持跨行（加上 s 修饰符的等效写法）
    .replace(/\*\*([\s\S]*?)\*\*/g, '<strong>$1</strong>')
    // 清理掉可能落单的成对星号（比如只剩结尾的 **）
    .replace(/\*\*/g, '')
    // 替换换行符 -> <br> 或者外层包裹段落
    .replace(/\n\n/g, '</p><p>')
    .replace(/\n/g, '<br>')
    // 替换无序列表 - item -> <li>item</li>
    .replace(/(?:<br>|^)- (.*?)(?=<br>|$)/g, '<li class="md-li">$1</li>')
    // 替换标题 ### Title -> <h4>Title</h4>
    .replace(/(?:<br>|^)### (.*?)(?=<br>|$)/g, '<h4 class="md-h4">$1</h4>')
    // 替换标题 #### Title -> <h5>Title</h5>
    .replace(/(?:<br>|^)#### (.*?)(?=<br>|$)/g, '<h5 class="md-h5">$1</h5>')

  return `<p>${html}</p>`.replace(/<p><\/p>/g, '')
}

const fillAiFormDefaults = () => {
  Object.assign(aiDecisionForm, createDefaultAiDecisionForm())
  applyRecentTankId(false)
}

const applyRecentTankId = (showMessage = true) => {
  const latestTankId = recordsList.value[0]?.tankId
  if (latestTankId) {
    aiDecisionForm.tankId = latestTankId
    if (showMessage) ElMessage.success(`已使用最近记录中的鱼缸 ID：${latestTankId}`)
    return
  }
  if (showMessage) ElMessage.warning('当前没有可用的最近记录鱼缸 ID')
}

// 是否来自缓存
const isAiDecisionFromCache = ref(false)

const handleAiDecision = async () => {
  await handleAiDecisionInternal(false)
}

// 重新生成（强制刷新缓存）
const handleRegenerateDecision = async () => {
  await handleAiDecisionInternal(true)
}

// 按重量投喂处理方法
const handleWeightFeed = async () => {
  if (!weightFeedForm.weight || weightFeedForm.weight <= 0) {
    ElMessage.warning('请输入有效的饲料重量')
    return
  }

  loadingWeightFeed.value = true
  try {
    const params = {
      weight: weightFeedForm.weight,
      immediate: weightFeedForm.immediate,
      countdown: weightFeedForm.immediate ? 0 : weightFeedForm.countdown
    }
    const result = await feedByWeight(weightFeedForm.servoId, params)
    ElMessage.success(result)
    // 刷新统计数据
    fetchTodayStats()
  } catch (error) {
    console.error('按重量投喂失败:', error)
    ElMessage.error('投喂失败，请重试')
  } finally {
    loadingWeightFeed.value = false
  }
}

// 重量变化时自动计算时间
const handleWeightChange = (val) => {
  if (val && val > 0) {
    const time = val * feedParams.value.timePerGram
    console.log(`饲料重量 ${val}g，计算投喂时间 ${time.toFixed(1)} 秒`)
  }
}

// 快速设置重量
const setQuickWeight = (weight) => {
  weightFeedForm.weight = weight
  handleWeightChange(weight)
  ElMessage.info(`已选择 ${weight}g 饲料，预计投喂 ${(weight * feedParams.value.timePerGram).toFixed(1)} 秒`)
}

// 重置重量投喂表单
const resetWeightFeedForm = () => {
  weightFeedForm.servoId = 1
  weightFeedForm.weight = 10
  weightFeedForm.immediate = true
  weightFeedForm.countdown = 5
  ElMessage.info('已重置为默认值')
}

// 获取投喂参数
const fetchFeedParams = async () => {
  try {
    const res = await getFeedCalculatorParams()
    if (res) {
      feedParams.value = { ...feedParams.value, ...res }
    }
  } catch (error) {
    console.error('获取投喂参数失败:', error)
  }
}

// 内部处理方法
const handleAiDecisionInternal = async (forceRefresh = false) => {
  if (!aiDecisionForm.tankId) {
    ElMessage.warning('请先填写鱼缸 ID')
    return
  }
  if (aiDecisionForm.blackFishCount + aiDecisionForm.goldFishCount <= 0) {
    ElMessage.warning('请至少填写一种鱼的数量')
    return
  }

  loadingAiDecision.value = true
  isAiDecisionFromCache.value = false
  try {
    const requestData = { ...aiDecisionForm, forceRefresh }
    const result = await analyzeFeedDecision(requestData)
    aiDecisionResult.value = result.data
    isAiDecisionFromCache.value = result.isCached
    if (result.isCached) {
      ElMessage.success('AI 投喂建议已生成（来自缓存）')
    } else {
      if (forceRefresh) {
        ElMessage.success('AI 投喂建议已重新生成')
      } else {
        ElMessage.success('AI 投喂建议已生成')
      }
    }
  } catch (error) {
    console.error('获取 AI 投喂建议失败:', error)
  } finally {
    loadingAiDecision.value = false
  }
}

// 获取今日统计数据
const fetchTodayStats = async () => {
  loadingStats.value = true
  try {
    const res = await getTodayStatistics()
    const data = res.data || res || {}
    
    // 后端返回格式为: { date: '...', servo1: { count: 5 }, servo2: { count: 3 }, total: 8 }
    const stats = {}
    if (data.servo1) stats['1'] = data.servo1.count || 0
    if (data.servo2) stats['2'] = data.servo2.count || 0
    if (data.total !== undefined) stats['total'] = data.total || 0
    
    todayStats.value = stats
    
    nextTick(() => {
      initPieChart()
    })
  } catch (error) {
    console.error('获取今日投喂统计失败:', error)
    // 错误已由拦截器处理
  } finally {
    loadingStats.value = false
  }
}

// 获取记录列表
const fetchRecords = async () => {
  loadingRecords.value = true
  recordsList.value = []
  try {
    let res
    if (currentTab.value === 'recent') {
      res = await getRecentRecords(20)
    } else if (currentTab.value === 'servo1') {
      res = await getServoRecords(1, 20)
    } else if (currentTab.value === 'servo2') {
      res = await getServoRecords(2, 20)
    }
    
    // 适配可能的返回结构
    recordsList.value = Array.isArray(res) ? res : (res?.data || [])
    
    nextTick(() => {
      initBarChart()
    })
  } catch (error) {
    console.error('获取投喂记录失败:', error)
  } finally {
    loadingRecords.value = false
  }
}

// 切换标签
const handleTabChange = () => {
  fetchRecords()
}

onMounted(() => {
  fetchTodayStats()
  fetchRecords()
  fetchFeedParams()
  window.addEventListener('resize', handleResize)
})

onUnmounted(() => {
  window.removeEventListener('resize', handleResize)
  if (pieChart) pieChart.dispose()
  if (barChart) barChart.dispose()
})
</script>

<style scoped>
.feed-records-page {
  padding: 24px;
  background-color: var(--surface-bg);
  min-height: calc(100vh - 60px);
}

.page-header {
  margin-bottom: 24px;
}

.page-title {
  font-size: 24px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 8px 0;
}

.page-subtitle {
  font-size: 14px;
  color: var(--text-secondary);
  margin: 0;
}

.statistics-row {
  margin-bottom: 24px;
}

.stat-card {
  border: none;
  border-radius: var(--radius-lg);
  overflow: hidden;
  position: relative;
  transition: transform var(--transition);
}

.stat-card:hover {
  transform: translateY(-4px);
}

/* 渐变卡片背景 */
.stat-card-1 {
  background: linear-gradient(135deg, #e0f2fe 0%, #bae6fd 100%);
}

.stat-card-2 {
  background: linear-gradient(135deg, #dcfce7 0%, #a7f3d0 100%);
}

.stat-card-total {
  background: linear-gradient(135deg, #fef08a 0%, #fde047 100%);
}

.empty-stat {
  background: linear-gradient(135deg, #f3f4f6 0%, #e5e7eb 100%);
}

.stat-content {
  display: flex;
  align-items: center;
  padding: 10px;
}

.stat-icon {
  width: 56px;
  height: 56px;
  border-radius: 50%;
  background-color: rgba(255, 255, 255, 0.6);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 28px;
  color: var(--teal-primary);
  margin-right: 20px;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.05);
}

.stat-card-2 .stat-icon {
  color: #10b981;
}

.stat-card-total .stat-icon {
  color: #eab308;
}

.stat-info {
  flex: 1;
}

.stat-value {
  font-size: 32px;
  font-weight: 700;
  color: var(--text-primary);
  line-height: 1.2;
}

.stat-unit {
  font-size: 16px;
  font-weight: 500;
  color: var(--text-secondary);
}

.stat-label {
  font-size: 14px;
  color: var(--text-secondary);
  margin-top: 4px;
  font-weight: 500;
}

.charts-row {
  margin-bottom: 24px;
}

.chart-card {
  border: 1px solid var(--border-light);
  border-radius: var(--radius-lg);
  background-color: var(--surface-card);
  height: 320px;
}

:deep(.chart-card .el-card__header) {
  padding: 12px 20px;
  border-bottom: 1px solid var(--border-light);
  background-color: rgba(0,0,0,0.02);
}

.card-header {
  font-weight: 600;
  color: var(--text-primary);
  font-size: 15px;
}

.chart-container {
  height: 250px;
  width: 100%;
}

.records-card {
  border: 1px solid var(--border-light);
  border-radius: var(--radius-lg);
  background-color: var(--surface-card);
}

.ai-decision-card {
  margin-bottom: 24px;
  border: 1px solid rgba(14, 165, 233, 0.15);
  border-radius: var(--radius-lg);
  overflow: hidden;
  background:
    radial-gradient(circle at top left, rgba(14, 165, 233, 0.14), transparent 34%),
    radial-gradient(circle at top right, rgba(16, 185, 129, 0.14), transparent 28%),
    linear-gradient(180deg, rgba(248, 250, 252, 0.96), rgba(255, 255, 255, 0.98));
}

.ai-header {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 16px;
}

.ai-title {
  margin: 0;
  font-size: 18px;
  color: var(--text-primary);
}

.ai-subtitle {
  margin: 6px 0 0;
  font-size: 13px;
  color: var(--text-secondary);
}

.ai-decision-layout {
  display: grid;
  grid-template-columns: minmax(360px, 1.05fr) minmax(320px, 1fr);
  gap: 20px;
}

.ai-form-panel,
.ai-result-panel {
  border-radius: var(--radius-lg);
  background: rgba(255, 255, 255, 0.72);
  border: 1px solid rgba(148, 163, 184, 0.18);
  padding: 20px;
  backdrop-filter: blur(10px);
}

.ai-panel-intro {
  display: flex;
  align-items: center;
  flex-wrap: wrap;
  gap: 10px;
  margin-bottom: 18px;
  font-size: 13px;
  color: var(--text-secondary);
}

.ai-panel-badge {
  padding: 4px 10px;
  border-radius: 999px;
  background: rgba(14, 165, 233, 0.12);
  color: #0284c7;
  font-weight: 600;
}

.ai-form :deep(.el-form-item) {
  margin-bottom: 18px;
}

.full-width {
  width: 100%;
}

.full-width :deep(.el-input-number) {
  width: 100%;
}

.ai-action-row {
  display: flex;
  flex-wrap: wrap;
  gap: 12px;
  margin-top: 6px;
}

.ai-result-top {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 16px;
  margin-bottom: 16px;
}

.result-kicker {
  font-size: 12px;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: #0284c7;
  font-weight: 700;
}

.result-time {
  margin-top: 6px;
  font-size: 13px;
  color: var(--text-secondary);
}

.recommend-grid {
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: 16px;
  margin: 18px 0;
}

.recommend-card {
  padding: 18px;
  border-radius: 20px;
  background: linear-gradient(135deg, rgba(14, 165, 233, 0.12), rgba(14, 165, 233, 0.04));
  border: 1px solid rgba(14, 165, 233, 0.18);
  display: flex;
  flex-direction: column;
  max-height: 250px;
}

.recommend-card-gold {
  background: linear-gradient(135deg, rgba(245, 158, 11, 0.16), rgba(250, 204, 21, 0.06));
  border-color: rgba(245, 158, 11, 0.2);
}

.recommend-title {
  font-size: 14px;
  color: var(--text-secondary);
  font-weight: 600;
}

.recommend-amount {
  margin-top: 10px;
  font-size: 30px;
  line-height: 1;
  font-weight: 700;
  color: var(--text-primary);
}

.recommend-meta {
  margin-top: 10px;
  font-size: 13px;
  color: var(--text-secondary);
  padding-bottom: 12px;
  border-bottom: 1px dashed rgba(0,0,0,0.06);
}

.recommend-scrollable {
  flex: 1;
  overflow-y: auto;
  padding-right: 4px;
}

/* 隐藏滚动条让卡片更干净，但保留滚动能力 */
.recommend-scrollable::-webkit-scrollbar {
  width: 4px;
}
.recommend-scrollable::-webkit-scrollbar-thumb {
  background: #cbd5e1;
  border-radius: 4px;
}

.recommend-reason,
.recommend-note {
  margin: 12px 0 0;
  font-size: 13px;
  line-height: 1.7;
  color: var(--text-regular);
}

.recommend-note {
  color: var(--text-secondary);
  background: rgba(0,0,0,0.03);
  padding: 8px;
  border-radius: 4px;
}

.ai-analysis-box {
  margin-top: 18px;
  padding: 18px;
  border-radius: 18px;
  background: rgba(241, 245, 249, 0.86);
  border: 1px solid rgba(148, 163, 184, 0.16);
  max-height: 300px;
  overflow-y: auto;
}

.analysis-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
  margin-bottom: 12px;
  font-weight: 600;
  color: var(--text-primary);
}

.analysis-body {
  margin: 0;
  color: var(--text-regular);
  line-height: 1.8;
  font-size: 13px;
}

/* Markdown 简易渲染样式 */
.md-content strong {
  color: var(--text-primary);
  font-weight: 600;
}
.md-content p {
  margin: 0 0 8px 0;
}
.md-content p:last-child {
  margin-bottom: 0;
}
.md-content .md-li {
  display: block;
  position: relative;
  padding-left: 14px;
  margin-bottom: 4px;
}
.md-content .md-li::before {
  content: "•";
  position: absolute;
  left: 0;
  color: var(--teal-primary);
  font-weight: bold;
}
.md-content .md-h4 {
  font-size: 15px;
  margin: 12px 0 8px 0;
  color: var(--text-primary);
  border-bottom: 1px solid var(--border-light);
  padding-bottom: 4px;
}
.md-content .md-h5 {
  font-size: 14px;
  margin: 8px 0 4px 0;
  color: var(--text-primary);
}

.ai-result-empty {
  min-height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
}

.records-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.custom-table {
  border-radius: var(--radius-md);
  overflow: hidden;
  border: 1px solid var(--border-light);
}

.duration-text {
  font-family: monospace;
  font-weight: 600;
  color: var(--teal-primary);
}

.time-cell {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  color: var(--text-regular);
}

.full-width {
  width: 100%;
}

:deep(.el-table__row) {
  transition: background-color 0.2s;
}

:deep(.el-table__row:hover) {
  background-color: var(--ocean-pale) !important;
}

:deep(.el-radio-button__inner) {
  border-color: var(--border-light);
}

@media (max-width: 992px) {
  .ai-decision-layout {
    grid-template-columns: 1fr;
  }

  .recommend-grid {
    grid-template-columns: 1fr;
  }
}

/* 按重量投喂卡片样式 */
.weight-feed-card {
  margin-bottom: 24px;
  border-radius: 12px;
  background: linear-gradient(135deg, #f0fdf4 0%, #dcfce7 100%);
  border: 1px solid #bbf7d0;
}

.weight-feed-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.weight-feed-title {
  font-size: 18px;
  font-weight: 600;
  color: #166534;
  margin: 0 0 6px 0;
  display: flex;
  align-items: center;
  gap: 8px;
}

.weight-feed-subtitle {
  font-size: 13px;
  color: #15803d;
  margin: 0;
}

.weight-feed-layout {
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 24px;
}

.weight-feed-form-panel {
  padding: 20px;
  background: white;
  border-radius: 8px;
  border: 1px solid #bbf7d0;
}

.weight-feed-intro {
  margin-bottom: 20px;
  padding: 12px 16px;
  background: #f0fdf4;
  border-radius: 8px;
  border-left: 4px solid #22c55e;
  font-size: 14px;
  color: #166534;
}

.weight-feed-badge {
  display: inline-block;
  padding: 2px 8px;
  background: #22c55e;
  color: white;
  border-radius: 4px;
  font-size: 12px;
  font-weight: 500;
  margin-right: 8px;
}

.weight-feed-form .el-form-item {
  margin-bottom: 16px;
}

.weight-feed-action-row {
  display: flex;
  gap: 12px;
  margin-top: 24px;
  padding-top: 20px;
  border-top: 1px dashed #bbf7d0;
}

.quick-weight-buttons {
  margin-top: 16px;
  padding-top: 16px;
  border-top: 1px dashed #bbf7d0;
  display: flex;
  align-items: center;
  gap: 12px;
}

.quick-label {
  font-size: 14px;
  color: #166534;
  font-weight: 500;
}

.weight-feed-info-panel {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.feed-params-info,
.feed-calculator-demo {
  padding: 20px;
  background: white;
  border-radius: 8px;
  border: 1px solid #bbf7d0;
}

.feed-params-info h4,
.feed-calculator-demo h4 {
  font-size: 16px;
  font-weight: 600;
  color: #166534;
  margin: 0 0 16px 0;
  padding-bottom: 12px;
  border-bottom: 2px solid #bbf7d0;
}

.param-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px 0;
  border-bottom: 1px solid #f0fdf4;
}

.param-item:last-child {
  border-bottom: none;
}

.param-label {
  font-size: 14px;
  color: #15803d;
}

.param-value {
  font-size: 14px;
  font-weight: 600;
  color: #166534;
}

.param-desc {
  margin-top: 12px;
  padding: 10px;
  background: #f0fdf4;
  border-radius: 6px;
  font-size: 13px;
  color: #166534;
  text-align: center;
}

.demo-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 12px 0;
  border-bottom: 1px solid #f0fdf4;
}

.demo-item:last-child {
  border-bottom: none;
}

.demo-weight {
  font-size: 16px;
  font-weight: 600;
  color: #166534;
  background: #f0fdf4;
  padding: 4px 12px;
  border-radius: 4px;
}

.demo-arrow {
  font-size: 14px;
  color: #22c55e;
  font-weight: 600;
}

.demo-time {
  font-size: 14px;
  color: #15803d;
  font-weight: 500;
}

@media (max-width: 992px) {
  .weight-feed-layout {
    grid-template-columns: 1fr;
  }
}
</style>
