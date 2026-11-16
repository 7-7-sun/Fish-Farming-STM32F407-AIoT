<template>
  <div class="alert-management">
    <div class="page-header">
      <div class="header-left">
        <h2 class="page-title">告警管理中心</h2>
        <p class="page-subtitle">监控鱼缸实时状态与管理告警规则</p>
      </div>
      <div class="header-actions">
        <el-form inline size="large" class="tank-selector">
          <el-form-item label="当前鱼缸">
            <el-select v-model="currentTankId" @change="handleTankChange" style="width: 120px">
              <el-option label="1号缸" :value="1" />
              <el-option label="2号缸" :value="2" />
              <el-option label="3号缸" :value="3" />
            </el-select>
          </el-form-item>
          <el-form-item>
            <el-button type="primary" :icon="Refresh" @click="fetchData" circle />
          </el-form-item>
        </el-form>
      </div>
    </div>

    <!-- 告警统计 -->
    <div class="statistics-section" v-loading="statsLoading">
      <el-row :gutter="24">
        <el-col :span="6">
          <div class="stat-card total-card">
            <div class="stat-icon-wrapper">
              <el-icon><Bell /></el-icon>
            </div>
            <div class="stat-content">
              <div class="stat-title">未解决总数</div>
              <div class="stat-value">{{ statistics.totalUnresolved || 0 }}</div>
            </div>
          </div>
        </el-col>
        <el-col :span="6">
          <div class="stat-card critical-card">
            <div class="stat-icon-wrapper">
              <el-icon><CircleCloseFilled /></el-icon>
            </div>
            <div class="stat-content">
              <div class="stat-title">严重告警</div>
              <div class="stat-value">{{ statistics.criticalCount || 0 }}</div>
            </div>
          </div>
        </el-col>
        <el-col :span="6">
          <div class="stat-card warning-card">
            <div class="stat-icon-wrapper">
              <el-icon><WarningFilled /></el-icon>
            </div>
            <div class="stat-content">
              <div class="stat-title">警告级别</div>
              <div class="stat-value">{{ statistics.warningCount || 0 }}</div>
            </div>
          </div>
        </el-col>
        <el-col :span="6">
          <div class="stat-card info-card">
            <div class="stat-icon-wrapper">
              <el-icon><InfoFilled /></el-icon>
            </div>
            <div class="stat-content">
              <div class="stat-title">普通提示</div>
              <div class="stat-value">{{ statistics.infoCount || 0 }}</div>
            </div>
          </div>
        </el-col>
      </el-row>
    </div>

    <div class="main-content-card">
      <el-tabs v-model="activeTab" class="custom-tabs">
        <!-- 未解决告警日志 -->
        <el-tab-pane name="logs">
          <template #label>
            <span class="tab-label">
              <el-icon><List /></el-icon>
              <span>未解决告警</span>
              <el-badge v-if="unresolvedAlerts.length" :value="unresolvedAlerts.length" class="tab-badge" type="danger" />
            </span>
          </template>
          
          <el-table 
            :data="unresolvedAlerts" 
            v-loading="alertsLoading" 
            style="width: 100%" 
            :header-cell-style="{ background: '#f8fafc', color: '#475569', fontWeight: '600' }"
            stripe>
            <el-table-column prop="alertId" label="ID" width="80" align="center" />
            <el-table-column prop="alertLevel" label="级别" width="120">
              <template #default="{ row }">
                <el-tag :type="getLevelTagType(row.alertLevel)" effect="light" class="level-tag">
                  <div class="tag-inner">
                    <span class="status-dot" :class="row.alertLevel.toLowerCase()"></span>
                    {{ row.alertLevel }}
                  </div>
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column prop="alertType" label="类型" width="150" />
            <el-table-column prop="alertMessage" label="告警信息" min-width="250" />
            <el-table-column prop="recordTime" label="记录时间" width="180">
              <template #default="{ row }">
                <div class="time-cell">
                  <el-icon><Clock /></el-icon>
                  <span>{{ formatDate(row.recordTime) }}</span>
                </div>
              </template>
            </el-table-column>
            <el-table-column label="操作" width="120" fixed="right" align="center">
              <template #default="{ row }">
                <el-popconfirm title="确定已解决该告警吗？" @confirm="handleResolve(row.alertId)" width="200">
                  <template #reference>
                    <el-button type="primary" link size="small" :icon="Check">标记解决</el-button>
                  </template>
                </el-popconfirm>
              </template>
            </el-table-column>
            <template #empty>
              <el-empty description="太棒了，当前没有未解决的告警！" :image-size="100" />
            </template>
          </el-table>
        </el-tab-pane>

        <!-- 告警规则配置 -->
        <el-tab-pane name="rules">
          <template #label>
            <span class="tab-label">
              <el-icon><Setting /></el-icon>
              <span>告警规则配置</span>
            </span>
          </template>

          <div class="rules-toolbar">
            <el-button type="primary" :icon="Plus" @click="handleCreateRule" class="create-btn">新建规则</el-button>
            <el-button :icon="RefreshLeft" @click="handleInitRules">初始化默认规则</el-button>
          </div>
          
          <el-table 
            :data="alertRules" 
            v-loading="rulesLoading" 
            style="width: 100%" 
            :header-cell-style="{ background: '#f8fafc', color: '#475569', fontWeight: '600' }">
            <el-table-column prop="ruleName" label="规则名称" min-width="150" />
            <el-table-column label="监测条件" min-width="200">
              <template #default="{ row }">
                <div class="condition-cell">
                  <el-tag size="small" type="info">{{ row.parameterName }}</el-tag>
                  <span class="operator">{{ row.conditionType }}</span>
                  <span class="value">{{ row.thresholdValue }}</span>
                </div>
              </template>
            </el-table-column>
            <el-table-column prop="alertLevel" label="触发级别" width="120">
              <template #default="{ row }">
                <el-tag :type="getLevelTagType(row.alertLevel)" effect="plain" size="small">{{ row.alertLevel }}</el-tag>
              </template>
            </el-table-column>
            <el-table-column prop="enabled" label="状态" width="100" align="center">
              <template #default="{ row }">
                <el-switch v-model="row.enabled" @change="handleToggleRule(row)" 
                  style="--el-switch-on-color: #10b981" />
              </template>
            </el-table-column>
            <el-table-column label="操作" width="150" fixed="right" align="center">
              <template #default="{ row }">
                <el-button type="primary" link :icon="Edit" @click="handleEditRule(row)">编辑</el-button>
                <el-popconfirm title="确定删除？" @confirm="handleDeleteRule(row.ruleId)">
                  <template #reference>
                    <el-button type="danger" link :icon="Delete">删除</el-button>
                  </template>
                </el-popconfirm>
              </template>
            </el-table-column>
          </el-table>
        </el-tab-pane>
      </el-tabs>
    </div>

    <!-- 告警规则表单弹窗 -->
    <el-dialog :title="isEdit ? '编辑规则' : '新建规则'" v-model="ruleDialogVisible" width="600px">
      <el-form :model="ruleForm" :rules="ruleRules" ref="ruleFormRef" label-width="120px">
        <el-form-item label="规则名称" prop="ruleName">
          <el-input v-model="ruleForm.ruleName" placeholder="例如：水温过高告警" />
        </el-form-item>
        <el-form-item label="监测参数" prop="parameterName">
          <el-input v-model="ruleForm.parameterName" placeholder="例如：temperature" />
        </el-form-item>
        <el-form-item label="条件与阈值">
          <el-row :gutter="10">
            <el-col :span="8">
              <el-select v-model="ruleForm.conditionType" placeholder="条件">
                <el-option label="大于 (>)" value=">" />
                <el-option label="小于 (<)" value="<" />
                <el-option label="等于 (=)" value="=" />
                <el-option label="不等于 (!=)" value="!=" />
              </el-select>
            </el-col>
            <el-col :span="16">
              <el-input-number v-model="ruleForm.thresholdValue" :step="0.1" style="width: 100%" />
            </el-col>
          </el-row>
        </el-form-item>
        <el-form-item label="最大值(可选)">
          <el-input-number v-model="ruleForm.maxValue" :step="0.1" />
        </el-form-item>
        <el-form-item label="告警级别" prop="alertLevel">
          <el-select v-model="ruleForm.alertLevel" placeholder="请选择">
            <el-option label="提示 (INFO)" value="INFO" />
            <el-option label="警告 (WARNING)" value="WARNING" />
            <el-option label="严重 (CRITICAL)" value="CRITICAL" />
          </el-select>
        </el-form-item>
        <el-form-item label="冷却时间(分)" prop="cooldownMinutes">
          <el-input-number v-model="ruleForm.cooldownMinutes" :min="0" />
        </el-form-item>
        <el-form-item label="通知渠道" prop="notificationChannels">
          <el-input v-model="ruleForm.notificationChannels" placeholder="例如：SYSTEM,EMAIL" />
        </el-form-item>
        <el-form-item label="消息模板" prop="alertMessageTemplate">
          <el-input type="textarea" v-model="ruleForm.alertMessageTemplate" placeholder="例如：水温异常，当前值为{value}" />
        </el-form-item>
        <el-form-item label="是否启用">
          <el-switch v-model="ruleForm.enabled" />
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="ruleDialogVisible = false">取消</el-button>
          <el-button type="primary" @click="handleSaveRule" :loading="savingRule">确定</el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import { 
  Warning, 
  Check, 
  Delete, 
  Edit, 
  Plus, 
  Refresh, 
  RefreshLeft,
  Bell,
  CircleCloseFilled,
  WarningFilled,
  InfoFilled,
  List,
  Setting,
  Clock
} from '@element-plus/icons-vue'
import {
  resolveAlert,
  updateAlertRule,
  deleteAlertRule,
  getAlertRules,
  createAlertRule,
  initAlertRules,
  getUnresolvedAlerts,
  getAlertStatistics
} from '../api/modules/alert'

// 基础状态
const activeTab = ref('logs')
const currentTankId = ref(1)

// 数据状态
const statistics = ref({})
const unresolvedAlerts = ref([])
const alertRules = ref([])

// 加载状态
const statsLoading = ref(false)
const alertsLoading = ref(false)
const rulesLoading = ref(false)

// 表单相关状态
const ruleDialogVisible = ref(false)
const isEdit = ref(false)
const savingRule = ref(false)
const ruleFormRef = ref(null)

const ruleForm = reactive({
  ruleId: null,
  tankId: 1,
  ruleName: '',
  parameterName: '',
  conditionType: '>',
  thresholdValue: 0,
  maxValue: 0,
  alertLevel: 'WARNING',
  enabled: true,
  cooldownMinutes: 30,
  notificationChannels: 'SYSTEM',
  alertMessageTemplate: ''
})

const ruleRules = {
  ruleName: [{ required: true, message: '请输入规则名称', trigger: 'blur' }],
  parameterName: [{ required: true, message: '请输入监测参数', trigger: 'blur' }],
  alertLevel: [{ required: true, message: '请选择告警级别', trigger: 'change' }]
}

// 格式化时间
const formatDate = (dateValue) => {
  if (!dateValue) return '-'
  
  // 如果是数组格式 [2026, 4, 19, 12, 9, 25]
  if (Array.isArray(dateValue) && dateValue.length >= 6) {
    const [year, month, day, hour, minute, second] = dateValue
    return `${year}-${String(month).padStart(2, '0')}-${String(day).padStart(2, '0')} ${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}:${String(second).padStart(2, '0')}`
  }
  
  // 如果是字符串格式
  if (typeof dateValue === 'string') {
    // 替换 T 为空格，确保格式一致
    const normalized = dateValue.replace('T', ' ')
    // 尝试匹配日期时间格式
    const match = normalized.match(/^(\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2}):(\d{2})/)
    if (match) {
      return `${match[1]}-${match[2]}-${match[3]} ${match[4]}:${match[5]}:${match[6]}`
    }
    // 尝试解析为 Date
    const date = new Date(dateValue)
    if (!isNaN(date.getTime())) {
      return `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')} ${String(date.getHours()).padStart(2, '0')}:${String(date.getMinutes()).padStart(2, '0')}:${String(date.getSeconds()).padStart(2, '0')}`
    }
  }
  
  return '-'
}

// 获取级别对应的颜色样式
const getLevelTagType = (level) => {
  const map = {
    'INFO': 'info',
    'WARNING': 'warning',
    'CRITICAL': 'danger',
    'ERROR': 'danger'
  }
  return map[level] || 'info'
}

const getLevelClass = (level) => {
  const l = level.toUpperCase()
  if (l.includes('CRITICAL') || l.includes('ERROR')) return 'stat-danger'
  if (l.includes('WARNING')) return 'stat-warning'
  if (l.includes('INFO')) return 'stat-info'
  return 'stat-primary'
}

// 获取全部数据
const fetchData = () => {
  fetchStatistics()
  fetchAlerts()
  fetchRules()
}

const handleTankChange = () => {
  fetchData()
}

// 接口调用：告警统计
const fetchStatistics = async () => {
  statsLoading.value = true
  try {
    const res = await getAlertStatistics({ tankId: currentTankId.value })
    // 如果返回体直接是对象
    statistics.value = res.data || res || {}
  } catch (error) {
    console.error('获取告警统计失败', error)
  } finally {
    statsLoading.value = false
  }
}

// 接口调用：未解决告警
const fetchAlerts = async () => {
  alertsLoading.value = true
  try {
    const res = await getUnresolvedAlerts({ tankId: currentTankId.value })
    // 兼容后端直接返回数组，或包含在 data / records 字段中
    unresolvedAlerts.value = res.data || res.records || (Array.isArray(res) ? res : [])
  } catch (error) {
    console.error('获取告警日志失败', error)
  } finally {
    alertsLoading.value = false
  }
}

// 接口调用：告警规则
const fetchRules = async () => {
  rulesLoading.value = true
  try {
    const res = await getAlertRules({ tankId: currentTankId.value })
    // 兼容后端直接返回数组，或包含在 data / records 字段中
    alertRules.value = res.data || res.records || (Array.isArray(res) ? res : [])
  } catch (error) {
    console.error('获取告警规则失败', error)
  } finally {
    rulesLoading.value = false
  }
}

// 接口调用：解决告警
const handleResolve = async (alertId) => {
  try {
    await resolveAlert(alertId)
    ElMessage.success('已标记为解决')
    fetchAlerts()
    fetchStatistics()
  } catch (error) {
    // 错误在 request.js 中拦截
  }
}

// 接口调用：初始化默认规则
const handleInitRules = async () => {
  try {
    await ElMessageBox.confirm('这将会为当前鱼缸初始化默认的告警规则，确定继续吗？', '提示', { type: 'warning' })
    await initAlertRules(currentTankId.value)
    ElMessage.success('初始化成功')
    fetchRules()
  } catch (error) {
    if (error !== 'cancel') {
      console.error(error)
    }
  }
}

// 接口调用：删除规则
const handleDeleteRule = async (ruleId) => {
  try {
    await deleteAlertRule(ruleId)
    ElMessage.success('删除成功')
    fetchRules()
  } catch (error) {
    //
  }
}

// 切换规则启用状态
const handleToggleRule = async (row) => {
  try {
    await updateAlertRule(row.ruleId, row)
    ElMessage.success(`${row.enabled ? '已启用' : '已禁用'}该规则`)
  } catch (error) {
    row.enabled = !row.enabled // 失败则回滚状态
  }
}

// 打开创建规则弹窗
const handleCreateRule = () => {
  isEdit.value = false
  Object.assign(ruleForm, {
    ruleId: null,
    tankId: currentTankId.value,
    ruleName: '',
    parameterName: '',
    conditionType: '>',
    thresholdValue: 0,
    maxValue: 0,
    alertLevel: 'WARNING',
    enabled: true,
    cooldownMinutes: 30,
    notificationChannels: 'SYSTEM',
    alertMessageTemplate: ''
  })
  ruleDialogVisible.value = true
  if (ruleFormRef.value) ruleFormRef.value.clearValidate()
}

// 打开编辑规则弹窗
const handleEditRule = (row) => {
  isEdit.value = true
  Object.assign(ruleForm, row)
  ruleDialogVisible.value = true
  if (ruleFormRef.value) ruleFormRef.value.clearValidate()
}

// 保存规则 (创建/更新)
const handleSaveRule = async () => {
  if (!ruleFormRef.value) return
  await ruleFormRef.value.validate(async (valid) => {
    if (valid) {
      savingRule.value = true
      try {
        ruleForm.tankId = currentTankId.value
        if (isEdit.value) {
          await updateAlertRule(ruleForm.ruleId, ruleForm)
          ElMessage.success('更新成功')
        } else {
          await createAlertRule(ruleForm)
          ElMessage.success('创建成功')
        }
        ruleDialogVisible.value = false
        fetchRules()
      } catch (error) {
        //
      } finally {
        savingRule.value = false
      }
    }
  })
}

onMounted(() => {
  fetchData()
})
</script>

<style scoped>
.alert-management {
  padding: 24px;
  background-color: #f1f5f9;
  min-height: 100vh;
}

/* 页面头部 */
.page-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 24px;
}
.page-title {
  margin: 0 0 8px 0;
  font-size: 24px;
  font-weight: 600;
  color: #1e293b;
}
.page-subtitle {
  margin: 0;
  font-size: 14px;
  color: #64748b;
}
.tank-selector {
  margin-bottom: 0;
}

/* 统计卡片 */
.statistics-section {
  margin-bottom: 24px;
}
.stat-card {
  background: #ffffff;
  border-radius: 12px;
  padding: 20px;
  display: flex;
  align-items: center;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.05);
  transition: transform 0.2s ease, box-shadow 0.2s ease;
  border: 1px solid #e2e8f0;
}
.stat-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
}
.stat-icon-wrapper {
  width: 56px;
  height: 56px;
  border-radius: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 28px;
  margin-right: 16px;
}
.stat-content {
  flex: 1;
}
.stat-title {
  font-size: 14px;
  color: #64748b;
  margin-bottom: 4px;
  font-weight: 500;
}
.stat-value {
  font-size: 28px;
  font-weight: 700;
  color: #0f172a;
  line-height: 1;
}

/* 卡片颜色主题 */
.total-card .stat-icon-wrapper { background: #eff6ff; color: #3b82f6; }
.critical-card .stat-icon-wrapper { background: #fef2f2; color: #ef4444; }
.warning-card .stat-icon-wrapper { background: #fffbeb; color: #f59e0b; }
.info-card .stat-icon-wrapper { background: #f0fdf4; color: #10b981; }

/* 主体内容区 */
.main-content-card {
  background: #ffffff;
  border-radius: 12px;
  padding: 24px;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.05);
  border: 1px solid #e2e8f0;
}

/* 标签页定制 */
.custom-tabs :deep(.el-tabs__nav-wrap::after) {
  height: 1px;
  background-color: #e2e8f0;
}
.custom-tabs :deep(.el-tabs__item) {
  font-size: 15px;
  padding: 0 24px;
}
.tab-label {
  display: flex;
  align-items: center;
  gap: 8px;
}
.tab-badge {
  margin-left: 4px;
}

/* 表格样式优化 */
.el-table {
  --el-table-border-color: #e2e8f0;
  --el-table-header-bg-color: #f8fafc;
}
.level-tag {
  border-radius: 6px;
  padding: 4px 12px;
}
.tag-inner {
  display: flex;
  align-items: center;
  gap: 6px;
  font-weight: 600;
}
.status-dot {
  width: 6px;
  height: 6px;
  border-radius: 50%;
}
.status-dot.critical { background-color: #ef4444; }
.status-dot.warning { background-color: #f59e0b; }
.status-dot.info { background-color: #10b981; }

.time-cell {
  display: flex;
  align-items: center;
  gap: 6px;
  color: #64748b;
}
.condition-cell {
  display: flex;
  align-items: center;
  gap: 8px;
}
.condition-cell .operator {
  color: #94a3b8;
  font-weight: bold;
}
.condition-cell .value {
  font-weight: 600;
  color: #0f172a;
}

.rules-toolbar {
  margin-bottom: 20px;
  display: flex;
  gap: 12px;
}
.create-btn {
  background-color: #0f172a;
  border-color: #0f172a;
}
.create-btn:hover {
  background-color: #1e293b;
  border-color: #1e293b;
}
</style>
