<template>
  <div class="security-management-page">
    <!-- 顶部标题区 -->
    <div class="page-header">
      <div class="header-left">
        <h1>用户管理</h1>
      </div>
      <div class="header-right">
        <el-button
          type="primary"
          class="provision-btn">
          <el-icon><UserFilled /></el-icon> 新增访问权限
        </el-button>
      </div>
    </div>

    <!-- 统计卡片区 -->
    <el-row
      :gutter="20"
      class="stats-row">
      <el-col :span="8">
        <el-card
          class="stat-card"
          shadow="hover">
          <div class="stat-icon-wrapper">
            <el-icon class="stat-icon"><UserGroup /></el-icon>
          </div>
          <div class="stat-content">
            <div class="stat-label">在线人数</div>
            <div class="stat-value">1</div>
          </div>
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card
          class="stat-card"
          shadow="hover">
          <div class="stat-icon-wrapper">
            <el-icon class="stat-icon"><Document /></el-icon>
          </div>
          <div class="stat-content">
            <div class="stat-label">待审批</div>
            <div class="stat-value">03</div>
          </div>
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card
          class="stat-card"
          shadow="hover">
          <div class="stat-icon-wrapper">
            <el-icon class="stat-icon"><Monitor /></el-icon>
          </div>
          <div class="stat-content">
            <div class="stat-label">系统在线率</div>
            <div class="stat-value">99. %</div>
          </div>
        </el-card>
      </el-col>
    </el-row>

    <!-- 操作员表格区 -->
    <el-card
      class="operator-card"
      shadow="hover">
      <template #header>
        <div class="card-header">
          <h3>人员登记表</h3>
          <div class="header-actions">
            <el-icon class="action-icon"><Filter /></el-icon>
            <el-icon class="action-icon"><Download /></el-icon>
          </div>
        </div>
      </template>

      <el-table
        :data="operators"
        v-loading="loading"
        style="width: 100%"
        :header-cell-style="{ background: '#f0f7fa', color: '#475569' }">
        <el-table-column
          prop="userIdentity"
          label="用户身份"
          min-width="180">
          <template #default="scope">
            <div class="user-identity">
              <el-avatar
                :size="36"
                :src="scope.row.avatar" />
              <div class="user-info">
                <div class="user-name">{{ scope.row.name }}</div>
                <div class="user-email">{{ scope.row.email }}</div>
              </div>
            </div>
          </template>
        </el-table-column>
        <el-table-column
          prop="securityRole"
          min-width="120">
          <template #default="scope">
            <el-tag :type="getRoleTagType(scope.row.securityRole)">
              {{ scope.row.securityRole }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column
          prop="gridStatus"
          label="网格状态"
          min-width="100"
          :filters="[
            { text: '在线', value: '在线' },
            { text: '离线', value: '离线' },
            { text: '已暂停', value: '已暂停' }
          ]"
          :filter-method="(value, row) => row.gridStatus === value">
          <template #default="scope">
            <div class="status-indicator">
              <span
                class="status-dot"
                :class="{
                  online: scope.row.gridStatus === '在线',
                  offline: scope.row.gridStatus === '离线',
                  suspended: scope.row.gridStatus === '已暂停',
                }"></span>
              {{ scope.row.gridStatus }}
            </div>
          </template>
        </el-table-column>
        <el-table-column
          prop="lastTelemetry"
          label="最近遥测"
          min-width="140" />
        <el-table-column
          label="操作"
          min-width="160">
          <template #default="scope">
            <div class="action-buttons">
              <el-button
                type="text"
                icon="Key"
                size="small" />
              <el-button
                type="text"
                icon="Edit"
                size="small" />
              <el-button
                type="text"
                :icon="scope.row.gridStatus === '已暂停' ? 'Refresh' : 'Delete'"
                size="small"
                :class="{ danger: scope.row.gridStatus !== '已暂停' }" />
            </div>
          </template>
        </el-table-column>
      </el-table>

      <!-- 分页 -->
      <div class="pagination-wrapper">
        <div class="pagination-info">显示 1 到 4 条，共 24 名操作员</div>
        <el-pagination
          v-model:current-page="currentPage"
          :page-size="4"
          :total="24"
          layout="prev, pager, next"
          small />
      </div>
    </el-card>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import {
  // UserGroup,
  Document,
  Monitor,
  BellFilled,
  ArrowUp,
  ArrowDown,
  Filter,
  Download,
  Key,
  Edit,
  Delete,
  Refresh,
  UserFilled,
} from '@element-plus/icons-vue'
import { getUsers } from '../api/modules/user'

// 假数据：操作员列表
const operators = ref([])
const loading = ref(false)

// 获取用户列表
const fetchUsers = async () => {
  loading.value = true
  try {
    const res = await getUsers() // 如果不传参数可能返回所有用户
    let data = res.data || res.records || res || []
    
    // 兼容如果只返回单个用户对象的情况
    if (!Array.isArray(data)) {
      data = [data]
    }
    
    operators.value = data.map(item => ({
      ...item,
      avatar: item.avatar || 'https://cube.elemecdn.com/0/88/03b0d39583f48206768a7534e55bcpng.png',
      name: item.username || item.name || '未知用户',
      email: item.email || '未提供',
      securityRole: item.role || item.securityRole || '系统管理员',
      gridStatus: item.status === 1 ? '在线' : (item.status === 0 ? '离线' : '在线'),
      lastTelemetry: item.lastLoginTime || item.updateTime || '刚刚'
    }))
  } catch (error) {
    console.error('获取用户列表失败', error)
  } finally {
    loading.value = false
  }
}

onMounted(() => {
  fetchUsers()
})

// 分页当前页
const currentPage = ref(1)

// 根据角色获取标签类型
const getRoleTagType = (role) => {
  switch (role) {
    case '首席研究员':
      return 'primary'
    case '系统管理员':
      return 'warning'
    case '工程师':
      return 'info'
    case '观察员':
      return 'success'
    default:
      return 'info'
  }
}
</script>

<style scoped>
.security-management-page {
  padding: 24px;
  background-color: var(--surface-bg);
  min-height: 100vh;
}

/* 顶部标题区 */
.page-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 24px;
}

.header-left .section-title {
  font-size: 14px;
  font-weight: 600;
  color: #909399;
  text-transform: uppercase;
  letter-spacing: 1px;
  margin-bottom: 4px;
}

.header-left h1 {
  font-size: 24px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 8px;
}

.header-left p {
  font-size: 14px;
  color: var(--text-secondary);
  line-height: 1.5;
  margin: 0;
}

.provision-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  font-weight: 500;
}

/* 统计卡片区 */
.stats-row {
  margin-bottom: 24px;
}

.stat-card {
  padding: 0 16px;
  border: none;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
}

.stat-icon-wrapper {
  width: 48px;
  height: 48px;
  border-radius: 50%;
  background-color: var(--teal-subtle);
  display: flex;
  align-items: center;
  justify-content: center;
  margin-right: 16px;
}

.stat-icon {
  font-size: 24px;
  color: var(--teal-primary);
}

.stat-icon.red {
  color: #f56c6c;
}

.stat-content {
  flex: 1;
}

.stat-label {
  font-size: 12px;
  color: #909399;
  margin-bottom: 4px;
}

.stat-value {
  font-size: 20px;
  font-weight: 700;
  color: var(--text-primary);
}

.stat-trend {
  display: flex;
  align-items: center;
  gap: 2px;
  font-size: 12px;
  color: #67c23a;
}

.stat-trend.red {
  color: #f56c6c;
}

.trend-icon {
  font-size: 14px;
}

/* 操作员表格区 */
.operator-card {
  border: none;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.card-header h3 {
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0;
}

.header-actions {
  display: flex;
  gap: 12px;
}

.action-icon {
  font-size: 18px;
  color: var(--text-secondary);
  cursor: pointer;
  transition: color var(--transition);
}

.action-icon:hover {
  color: var(--teal-primary);
}

/* 表格内用户身份列 */
.user-identity {
  display: flex;
  align-items: center;
  gap: 8px;
}

.user-info {
  display: flex;
  flex-direction: column;
}

.user-name {
  font-size: 14px;
  font-weight: 500;
  color: var(--text-primary);
}

.user-email {
  font-size: 12px;
  color: #909399;
}

/* 安全角色标签 */
.el-tag {
  font-size: 12px;
  padding: 2px 8px;
}

/* 网格状态指示器 */
.status-indicator {
  display: flex;
  align-items: center;
  gap: 6px;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}

.status-dot.online {
  background-color: #67c23a;
}

.status-dot.offline {
  background-color: #909399;
}

.status-dot.suspended {
  background-color: #f56c6c;
}

/* 操作按钮 */
.action-buttons {
  display: flex;
  gap: 8px;
}

.action-buttons .el-button {
  padding: 0;
}

.action-buttons .danger {
  color: #f56c6c;
}

/* 分页区 */
.pagination-wrapper {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-top: 16px;
}

.pagination-info {
  font-size: 12px;
  color: #909399;
}

.el-pagination {
  --el-pagination-bg-color: transparent;
}
</style>
