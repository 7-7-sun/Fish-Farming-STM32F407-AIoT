<template>
  <div class="env-ranges-page">
    <!-- 顶部标题区 -->
    <div class="page-header">
      <div class="header-left">
        <h2 class="page-title">环境参数范围管理</h2>
        <p class="page-subtitle">设定并监控鱼缸各项环境指标的适宜区间与自动化控制策略</p>
      </div>
      <div class="header-actions">
        <el-form inline size="large" class="tank-selector">
          <el-form-item label="当前鱼缸">
            <el-select v-model="currentTankId" @change="fetchData" style="width: 120px">
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

    <!-- 主体内容卡片 -->
    <el-card class="main-content-card" shadow="never">
      <div class="toolbar">
        <el-button type="primary" :icon="Plus" @click="handleCreate" class="create-btn">新增参数范围</el-button>
        <el-button :icon="Document" @click="handleLoadDefault">加载默认范围</el-button>
      </div>

      <el-table 
        :data="ranges" 
        v-loading="loading" 
        style="width: 100%"
        :header-cell-style="{ background: '#f8fafc', color: '#475569', fontWeight: '600' }"
        stripe>
        <el-table-column prop="id" label="ID" width="80" align="center" />
        
        <el-table-column prop="parameterName" label="参数名称" width="150">
          <template #default="{ row }">
            <div class="param-name">
              <el-tag size="small" type="info" effect="plain">
                {{ getParameterDisplayName(row.parameterName) }}
              </el-tag>
            </div>
          </template>
        </el-table-column>
        
        <el-table-column label="目标理想值" width="120" align="center">
          <template #default="{ row }">
            <span class="ideal-value">{{ row.idealValue }}</span>
          </template>
        </el-table-column>

        <el-table-column label="安全区间 (Min - Max)" min-width="200">
          <template #default="{ row }">
            <div class="range-bar-wrapper">
              <span class="range-text min">{{ row.minValue }}</span>
              <div class="range-track">
                <div class="range-fill"></div>
              </div>
              <span class="range-text max">{{ row.maxValue }}</span>
            </div>
          </template>
        </el-table-column>

        <el-table-column prop="autoControlEnabled" label="自动化控制" width="120" align="center">
          <template #default="{ row }">
            <el-switch 
              v-model="row.autoControlEnabled" 
              @change="handleToggleAutoControl(row)"
              style="--el-switch-on-color: #10b981" />
          </template>
        </el-table-column>

        <el-table-column label="操作" width="150" fixed="right" align="center">
          <template #default="{ row }">
            <el-button type="primary" link :icon="Edit" @click="handleEdit(row)">编辑</el-button>
            <el-popconfirm title="确定删除该参数范围？" @confirm="handleDelete(row.id)">
              <template #reference>
                <el-button type="danger" link :icon="Delete">删除</el-button>
              </template>
            </el-popconfirm>
          </template>
        </el-table-column>
        <template #empty>
          <el-empty description="当前鱼缸暂未配置环境参数范围" :image-size="100" />
        </template>
      </el-table>
    </el-card>

    <!-- 新增/编辑弹窗 -->
    <el-dialog :title="isEdit ? '编辑环境参数范围' : '新增环境参数范围'" v-model="dialogVisible" width="500px">
      <el-form :model="form" :rules="rules" ref="formRef" label-width="110px">
        <el-form-item label="参数名称" prop="parameterName">
          <el-input v-model="form.parameterName" placeholder="例如：temperature, ph, oxygen" :disabled="isEdit" />
        </el-form-item>
        
        <el-form-item label="理想目标值" prop="idealValue">
          <el-input-number v-model="form.idealValue" :step="0.1" style="width: 100%" />
        </el-form-item>
        
        <el-form-item label="安全区间下限" prop="minValue">
          <el-input-number v-model="form.minValue" :step="0.1" style="width: 100%" />
        </el-form-item>
        
        <el-form-item label="安全区间上限" prop="maxValue">
          <el-input-number v-model="form.maxValue" :step="0.1" style="width: 100%" />
        </el-form-item>
        
        <el-form-item label="启用自动控制">
          <el-switch v-model="form.autoControlEnabled" style="--el-switch-on-color: #10b981" />
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="dialogVisible = false">取消</el-button>
          <el-button type="primary" @click="handleSubmit" :loading="saving">确定</el-button>
        </span>
      </template>
    </el-dialog>

  </div>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import { Refresh, Plus, Edit, Delete, Document } from '@element-plus/icons-vue'
import { 
  createEnvRange, 
  updateEnvRange, 
  toggleAutoControl, 
  getTankEnvRanges, 
  deleteEnvRange,
  getDefaultEnvRanges
} from '../api/modules/environment'

const currentTankId = ref(1)
const ranges = ref([])
const loading = ref(false)

// 弹窗表单相关
const dialogVisible = ref(false)
const isEdit = ref(false)
const saving = ref(false)
const formRef = ref(null)

const form = reactive({
  id: null,
  tankId: 1,
  parameterName: '',
  minValue: 0,
  maxValue: 0,
  idealValue: 0,
  autoControlEnabled: true
})

const rules = {
  parameterName: [{ required: true, message: '请输入参数名称', trigger: 'blur' }],
  minValue: [{ required: true, message: '请输入最小值', trigger: 'blur' }],
  maxValue: [{ required: true, message: '请输入最大值', trigger: 'blur' }],
  idealValue: [{ required: true, message: '请输入理想值', trigger: 'blur' }]
}

// 获取参数中文名称
const getParameterDisplayName = (paramName) => {
  const paramMap = {
    'temperature': '温度',
    'phValue': 'pH值',
    'turbidity': '浊度',
    'dissolvedOxygen': '溶解氧'
  }
  return paramMap[paramName] || paramName
}

// 获取数据
const fetchData = async () => {
  loading.value = true
  try {
    const res = await getTankEnvRanges(currentTankId.value)
    console.log('API返回数据:', res) // 调试日志
    ranges.value = res.data || res.records || res || []
    console.log('处理后的数据:', ranges.value) // 调试日志
  } catch (error) {
    console.error('获取参数范围失败', error)
  } finally {
    loading.value = false
  }
}

// 加载默认范围
const handleLoadDefault = async () => {
  try {
    const res = await getDefaultEnvRanges()
    console.log('默认配置返回:', res) // 调试日志
    let defaultData = res.data || res.records || res || []
    
    // 如果是对象格式，转换为数组
    if (typeof defaultData === 'object' && !Array.isArray(defaultData)) {
      defaultData = Object.values(defaultData)
    }
    
    console.log('转换后的默认数据:', defaultData) // 调试日志
    
    if (defaultData.length > 0) {
      ElMessageBox.confirm('这将会加载系统默认参数范围，仅供参考，确定查看吗？', '提示', { type: 'info' })
        .then(() => {
          ranges.value = defaultData
          ElMessage.success('已展示默认配置')
        }).catch(() => {})
    } else {
      ElMessage.warning('系统暂无默认配置')
    }
  } catch (error) {
    console.error('获取默认配置失败', error)
  }
}

// 新增
const handleCreate = () => {
  isEdit.value = false
  Object.assign(form, {
    id: null,
    tankId: currentTankId.value,
    parameterName: '',
    minValue: 0,
    maxValue: 0,
    idealValue: 0,
    autoControlEnabled: true
  })
  dialogVisible.value = true
  if (formRef.value) formRef.value.clearValidate()
}

// 编辑
const handleEdit = (row) => {
  isEdit.value = true
  Object.assign(form, row)
  dialogVisible.value = true
  if (formRef.value) formRef.value.clearValidate()
}

// 提交表单 (新增/更新)
const handleSubmit = async () => {
  if (!formRef.value) return
  await formRef.value.validate(async (valid) => {
    if (valid) {
      // 前端简单校验
      if (form.minValue >= form.maxValue) {
        ElMessage.warning('区间下限必须小于上限')
        return
      }
      
      saving.value = true
      try {
        form.tankId = currentTankId.value
        if (isEdit.value) {
          await updateEnvRange(form)
          ElMessage.success('更新成功')
        } else {
          await createEnvRange(form)
          ElMessage.success('新增成功')
        }
        dialogVisible.value = false
        fetchData()
      } catch (error) {
        // Error handled in request interceptor
      } finally {
        saving.value = false
      }
    }
  })
}

// 切换自动控制状态
const handleToggleAutoControl = async (row) => {
  try {
    // 接口是 PATCH /api/environment-ranges/{id}/toggle-auto-control
    // 需要传递 { enabled: true/false }
    await toggleAutoControl(row.id, row.autoControlEnabled)
    ElMessage.success(`${row.autoControlEnabled ? '已开启' : '已关闭'}自动控制`)
  } catch (error) {
    row.autoControlEnabled = !row.autoControlEnabled // 回滚状态
    console.error('切换自动控制失败', error)
    ElMessage.error('切换自动控制失败')
  }
}

// 删除
const handleDelete = async (id) => {
  try {
    await deleteEnvRange(id)
    ElMessage.success('删除成功')
    fetchData()
  } catch (error) {
    console.error('删除失败', error)
  }
}

onMounted(() => {
  fetchData()
})
</script>

<style scoped>
.env-ranges-page {
  padding: 24px;
  background-color: #f1f5f9;
  min-height: 100vh;
}

/* 顶部标题区 */
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

/* 内容卡片 */
.main-content-card {
  border-radius: 12px;
  border: 1px solid #e2e8f0;
}

.toolbar {
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

/* 表格内样式 */
.ideal-value {
  font-size: 16px;
  font-weight: 700;
  color: #10b981;
}

/* 范围区间模拟条 */
.range-bar-wrapper {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 0 10px;
}
.range-text {
  font-size: 13px;
  font-weight: 600;
  width: 36px;
  text-align: center;
}
.range-text.min { color: #3b82f6; }
.range-text.max { color: #ef4444; }

.range-track {
  flex: 1;
  height: 8px;
  background: #e2e8f0;
  border-radius: 4px;
  position: relative;
  overflow: hidden;
}
.range-fill {
  position: absolute;
  left: 20%;
  right: 20%;
  height: 100%;
  background: linear-gradient(90deg, #3b82f6, #10b981, #ef4444);
  border-radius: 4px;
}

:deep(.el-table__row) {
  height: 64px;
}
</style>
