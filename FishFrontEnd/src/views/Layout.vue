<template>
  <div class="dashboard-container">
    <el-aside
      width="260px"
      class="sidebar">
      <div class="logo-container">
        <div class="logo-icon">
          <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M2 12c.6.5 1.2 1 2.5 1 2.5 0 2.5-2 5-2 2.6 0 2.4 2 5 2 2.5 0 2.5-2 5-2 1.3 0 1.9.5 2.5 1" />
            <path d="M2 7c.6.5 1.2 1 2.5 1 2.5 0 2.5-2 5-2 2.6 0 2.4 2 5 2 2.5 0 2.5-2 5-2 1.3 0 1.9.5 2.5 1" />
          </svg>
        </div>
        <div>
          <div class="logo-main">水下观察者</div>
          <div class="logo-sub">智能水产分析系统</div>
        </div>
      </div>

      <el-menu
        :default-active="activeMenu"
        class="nav-menu"
        background-color="transparent"
        text-color="rgba(224,242,254,0.7)"
        active-text-color="#22d3ee"
        :router="true"
        @select="handleMenuSelect">
        <el-menu-item index="/monitoring" class="nav-item">
          <el-icon><VideoCameraFilled /></el-icon>
          <span>实时监控</span>
        </el-menu-item>
        <el-menu-item index="/weather" class="nav-item">
          <el-icon><Cloudy /></el-icon>
          <span>实时天气</span>
        </el-menu-item>
        <el-menu-item index="/aiAssistant" class="nav-item">
          <el-icon><ChatLineSquare /></el-icon>
          <span>AI助手</span>
        </el-menu-item>
        <el-menu-item index="/dataAnalysis" class="nav-item">
          <el-icon><DataAnalysis /></el-icon>
          <span>自动化控制</span>
        </el-menu-item>
        <el-menu-item index="/environmentRanges" class="nav-item">
          <el-icon><Odometer /></el-icon>
          <span>环境参数范围</span>
        </el-menu-item>

        <el-menu-item index="/feedRecords" class="nav-item">
          <el-icon><Dish /></el-icon>
          <span>投喂记录</span>
        </el-menu-item>
        <el-menu-item index="/fishAdvice" class="nav-item">
          <el-icon style="visibility: hidden;"><ChatDotRound /></el-icon>
          <span>养鱼建议</span>
        </el-menu-item>
        <el-menu-item index="/alertManagement" class="nav-item">
          <el-icon><Warning /></el-icon>
          <span>告警管理</span>
        </el-menu-item>
        <el-menu-item index="/userManagement" class="nav-item">
          <el-icon><UserFilled /></el-icon>
          <span>用户管理</span>
        </el-menu-item>
      </el-menu>

      <div class="user-info" @click="handleLogout">
        <el-avatar
          :size="38"
          src="https://cube.elemecdn.com/3/7c/3ea6beec64369c2642b92c6726f1epng.png" />
        <div class="user-details">
          <div class="user-name">{{ userStore.userInfo?.username || '管理员' }}</div>
          <div class="user-email">{{ userStore.userInfo?.email || 'admin@aquatic.cn' }}</div>
        </div>
        <el-icon class="logout-icon"><SwitchButton /></el-icon>
      </div>
    </el-aside>

    <div class="main-content">
      <header class="main-header">
        <div class="header-left">
          <div class="date-display">
            <span class="day">{{ currentDay }}</span>
            <div class="date-right">
              <span class="month">{{ currentMonth }}</span>
              <span class="year">{{ currentYear }}</span>
            </div>
          </div>
          <div class="header-title">
            <el-breadcrumb separator="/">
              <el-breadcrumb-item v-if="currentRouteTitle && currentRouteTitle !== '监控'">
                {{ currentRouteTitle }}
              </el-breadcrumb-item>
            </el-breadcrumb>
          </div>
        </div>
        <div class="header-right">
          <el-autocomplete
            ref="searchInputRef"
            v-model="searchQuery"
            :fetch-suggestions="querySearch"
            placeholder="搜索页面或功能 (如: 投喂)..."
            class="search-input"
            :prefix-icon="Search"
            clearable
            @select="handleSelect"
            popper-class="global-search-popper"
            value-key="title"
          >
            <template #default="{ item }">
              <div class="search-item">
                <div class="search-item-content">
                  <div class="search-item-title">{{ item.title }}</div>
                  <div class="search-item-desc">{{ item.desc }}</div>
                </div>
              </div>
            </template>
          </el-autocomplete>
        </div>
      </header>

      <div class="nav-tabs-container" v-if="visitedViews.length > 0">
        <el-tabs
          v-model="activeTab"
          type="card"
          closable
          @tab-remove="removeTab"
          @tab-click="clickTab"
        >
          <el-tab-pane
            v-for="item in visitedViews"
            :key="item.path"
            :label="item.title"
            :name="item.path"
          />
        </el-tabs>
      </div>

      <main class="content-area">
        <router-view v-slot="{ Component }">
          <transition name="fade-transform" mode="out-in">
            <component :is="Component" :key="route.path" />
          </transition>
        </router-view>
      </main>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted, watch } from 'vue'
import {
  Setting,
  Search,
  SwitchButton,
  ChatDotRound
} from '@element-plus/icons-vue'
import { useRouter, useRoute } from 'vue-router'
import { useUserStore } from '../store/user'

const router = useRouter()
const route = useRoute()
const userStore = useUserStore()

const searchInputRef = ref(null)
const searchQuery = ref('')
const currentDate = ref(new Date())



const searchOptions = [
  { title: '实时监控', path: '/monitoring', desc: '查看水下摄像头实时画面', keywords: ['jiankong', '视频', '摄像头', '实时', '画面'] },
  { title: '实时天气', path: '/weather', desc: '本地天气与气象数据', keywords: ['tianqi', 'weather', '温度', '湿度', '气象'] },
  { title: 'AI助手', path: '/aiAssistant', desc: '智能水产养殖问答助手', keywords: ['ai', 'zhushou', '问答', '聊天', '机器人'] },
  { title: '自动化控制', path: '/dataAnalysis', desc: '设备自动化运行状态与控制', keywords: ['zidonghua', 'kongzhi', '数据', '控制', '设备'] },
  { title: '环境参数范围', path: '/environmentRanges', desc: '水质参数阈值设置', keywords: ['huanjing', 'canshu', '阈值', '范围', '水质'] },

  { title: '投喂记录', path: '/feedRecords', desc: '历史投喂数据与计划', keywords: ['touwei', 'jilu', '喂食', '饲料'] },
  { title: '养鱼建议', path: '/fishAdvice', desc: 'AI智能养鱼建议与指导', keywords: ['yangyu', 'jianyi', '建议', '养鱼', '指导'] },
  { title: '用户管理', path: '/userManagement', desc: '系统账户与权限管理', keywords: ['yonghu', 'quanxian', '账号', '管理'] },
  { title: '告警管理', path: '/alertManagement', desc: '异常报警记录与处理', keywords: ['gaojing', 'baojing', '异常', '警告', '处理'] },
]

const querySearch = (queryString, cb) => {
  const results = queryString
    ? searchOptions.filter(createFilter(queryString))
    : searchOptions
  cb(results)
}

const createFilter = (queryString) => {
  const lowerQuery = queryString.toLowerCase()
  return (item) => {
    return (
      item.title.toLowerCase().includes(lowerQuery) ||
      item.desc.toLowerCase().includes(lowerQuery) ||
      item.keywords.some(k => k.toLowerCase().includes(lowerQuery))
    )
  }
}

const handleSelect = (item) => {
  router.push(item.path)
  searchQuery.value = ''
  searchInputRef.value?.blur()
}

const handleKeyDown = (e) => {
  if ((e.metaKey || e.ctrlKey) && e.key === 'k') {
    e.preventDefault()
    searchInputRef.value?.focus()
  }
}

const currentDay = computed(() => {
  return currentDate.value.getDate().toString().padStart(2, '0')
})

const currentMonth = computed(() => {
  const month = currentDate.value.toLocaleString('zh-CN', { month: 'short' })
  return month.replace('月', '').toUpperCase()
})

const currentYear = computed(() => {
  return currentDate.value.getFullYear()
})

const activeMenu = computed(() => {
  return route.path
})

const currentRouteTitle = computed(() => {
  return route.meta.title || '未知页面'
})

const activeTab = ref('')
const visitedViews = ref([])

watch(
  () => route.path,
  (newPath) => {
    activeTab.value = newPath
    if (!visitedViews.value.find(v => v.path === newPath)) {
      visitedViews.value.push({
        path: newPath,
        title: route.meta.title || '未知页面'
      })
    }
  },
  { immediate: true }
)

const removeTab = (targetName) => {
  const tabs = visitedViews.value
  let activeName = activeTab.value

  if (activeName === targetName) {
    tabs.forEach((tab, index) => {
      if (tab.path === targetName) {
        const nextTab = tabs[index + 1] || tabs[index - 1]
        if (nextTab) {
          activeName = nextTab.path
        } else {
          activeName = '/monitoring'
        }
      }
    })
  }

  activeTab.value = activeName
  visitedViews.value = tabs.filter((tab) => tab.path !== targetName)
  
  if (visitedViews.value.length === 0) {
    router.push('/monitoring')
  } else if (route.path !== activeName) {
    router.push(activeName)
  }
}

const clickTab = (tab) => {
  router.push(tab.props.name)
}

const handleMenuSelect = (index) => {
  router.push(index)
}

const handleLogout = () => {
  localStorage.removeItem('token')
  sessionStorage.removeItem('token')
  router.push('/login')
}

onMounted(() => {
  window.addEventListener('keydown', handleKeyDown)
})

onUnmounted(() => {
  window.removeEventListener('keydown', handleKeyDown)
})
</script>

<style scoped>
.dashboard-container {
  display: flex;
  height: 100vh;
  background-color: var(--surface-bg);
}

.sidebar {
  background: linear-gradient(180deg, var(--ocean-dark) 0%, var(--ocean-deep) 100%);
  display: flex;
  flex-direction: column;
  padding: 24px 0;
  color: #e0f2fe;
  position: relative;
  z-index: 10;
  box-shadow: 2px 0 16px rgba(10, 22, 40, 0.25);
  overflow: hidden;
}

.sidebar::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background:
    radial-gradient(ellipse at 20% 80%, rgba(8, 145, 178, 0.08) 0%, transparent 60%),
    radial-gradient(ellipse at 80% 20%, rgba(6, 182, 212, 0.05) 0%, transparent 50%);
  pointer-events: none;
}

.logo-container {
  padding: 0 24px 28px;
  border-bottom: 1px solid rgba(8, 145, 178, 0.15);
  margin-bottom: 20px;
  display: flex;
  align-items: center;
  gap: 14px;
  position: relative;
}

.logo-icon {
  width: 44px;
  height: 44px;
  background: linear-gradient(135deg, var(--teal-primary), var(--teal-light));
  border-radius: var(--radius-sm);
  display: flex;
  align-items: center;
  justify-content: center;
  color: #fff;
  flex-shrink: 0;
}

.logo-main {
  font-size: 18px;
  font-weight: 700;
  color: #ffffff;
  letter-spacing: 0.5px;
}

.logo-sub {
  font-size: 11px;
  color: rgba(148, 200, 224, 0.6);
  font-weight: 500;
  letter-spacing: 1.5px;
  text-transform: uppercase;
  margin-top: 2px;
}

.nav-menu {
  flex: 1;
  border: none;
  background: transparent !important;
  position: relative;
  overflow-y: auto;
  overflow-x: hidden;
}

.nav-menu::-webkit-scrollbar {
  width: 4px;
}

.nav-menu::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.1);
  border-radius: 2px;
}

.nav-menu::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.2);
}

:deep(.el-menu-item) {
  height: 46px;
  line-height: 46px;
  margin: 3px 12px;
  border-radius: var(--radius-sm);
  transition: all var(--transition);
  font-size: 14px;
}

:deep(.el-menu-item:hover) {
  background-color: rgba(8, 145, 178, 0.1) !important;
}

:deep(.el-menu-item.is-active) {
  background: linear-gradient(135deg, rgba(8, 145, 178, 0.2), rgba(6, 182, 212, 0.12)) !important;
  color: var(--teal-light) !important;
  font-weight: 600;
  box-shadow: inset 3px 0 0 var(--teal-primary);
}

:deep(.el-menu-item .el-icon) {
  font-size: 19px;
  margin-right: 12px;
}

.user-info {
  display: flex;
  align-items: center;
  padding: 14px 18px;
  border-top: 1px solid rgba(8, 145, 178, 0.12);
  background: rgba(8, 145, 178, 0.06);
  margin: 8px 12px 0;
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: background var(--transition);
  position: relative;
}

.user-info:hover {
  background: rgba(8, 145, 178, 0.12);
}

.user-details {
  flex: 1;
  margin-left: 12px;
}

.user-name {
  font-weight: 600;
  color: #ffffff;
  font-size: 13px;
  margin-bottom: 1px;
}

.user-email {
  color: rgba(148, 200, 224, 0.5);
  font-size: 11px;
}

.logout-icon {
  color: rgba(148, 200, 224, 0.4);
  cursor: pointer;
  transition: color var(--transition);
  font-size: 18px;
}

.logout-icon:hover {
  color: var(--teal-light);
}

.main-content {
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.main-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 32px;
  background-color: var(--surface-card);
  border-bottom: 1px solid var(--border-light);
  box-shadow: var(--shadow-sm);
  z-index: 5;
}

.header-left {
  display: flex;
  align-items: center;
  gap: 24px;
}

.date-display {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 14px;
  background: var(--teal-subtle);
  border: 1px solid rgba(8, 145, 178, 0.1);
  border-radius: var(--radius-sm);
  min-width: 120px;
}

.day {
  font-size: 26px;
  font-weight: 800;
  color: var(--teal-primary);
  line-height: 1;
}

.date-right {
  display: flex;
  flex-direction: column;
  line-height: 1.2;
}

.month {
  font-size: 13px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
}

.year {
  font-size: 11px;
  color: var(--text-muted);
}

.header-title {
  display: flex;
  align-items: center;
}

:deep(.el-breadcrumb__inner) {
  color: var(--text-secondary);
  font-weight: 500;
  font-size: 14px;
}

:deep(.el-breadcrumb__inner.is-link),
:deep(.el-breadcrumb__inner a) {
  color: var(--text-muted);
  font-weight: 400;
  transition: color var(--transition);
}

:deep(.el-breadcrumb__inner.is-link:hover),
:deep(.el-breadcrumb__inner a:hover) {
  color: var(--teal-primary);
}

:deep(.el-breadcrumb__item:last-child .el-breadcrumb__inner) {
  color: var(--text-primary);
  font-weight: 600;
}

.header-right {
  display: flex;
  align-items: center;
  gap: 14px;
}

.search-input {
  width: 260px;
}

:deep(.search-input .el-input__wrapper) {
  background-color: var(--ocean-pale);
  border: 1px solid var(--border-light);
  border-radius: 20px;
  padding: 0 16px;
  box-shadow: none;
  transition: all var(--transition);
}

:deep(.search-input .el-input__wrapper:hover),
:deep(.search-input .el-input__wrapper.is-focus) {
  border-color: var(--teal-primary);
  box-shadow: 0 0 0 3px var(--teal-glow);
}

:deep(.search-input .el-input__inner) {
  color: var(--text-secondary);
}

.search-item {
  padding: 4px 0;
}

.search-item-content {
  display: flex;
  flex-direction: column;
  line-height: 1.2;
}

.search-item-title {
  font-size: 14px;
  font-weight: 500;
  color: var(--text-primary);
  margin-bottom: 4px;
}

.search-item-desc {
  font-size: 12px;
  color: var(--text-muted);
}

.header-icon {
  :deep(.el-badge__content) {
    background-color: var(--coral-red);
    border: 2px solid var(--surface-card);
  }
}

.nav-tabs-container {
  background-color: var(--surface-card);
  padding: 10px 24px 0;
  border-bottom: 1px solid var(--border-light);
  box-shadow: 0 4px 12px rgba(10, 22, 40, 0.03);
  z-index: 4;
  position: relative;
}

:deep(.el-tabs__header) {
  margin: 0;
  border-bottom: none;
}

:deep(.el-tabs--card > .el-tabs__header .el-tabs__nav) {
  border: none;
  display: flex;
  gap: 6px;
}

:deep(.el-tabs--card > .el-tabs__header .el-tabs__item) {
  border: 1px solid var(--border-light) !important;
  border-bottom: none !important;
  border-radius: var(--radius-sm) var(--radius-sm) 0 0;
  background-color: var(--surface-bg);
  color: var(--text-secondary);
  height: 34px;
  line-height: 34px;
  transition: all var(--transition);
  font-size: 13px;
  padding: 0 16px !important;
}

:deep(.el-tabs--card > .el-tabs__header .el-tabs__item.is-active) {
  background-color: var(--surface-card);
  color: var(--teal-primary);
  border-bottom-color: var(--surface-card) !important;
  font-weight: 600;
  box-shadow: inset 0 2px 0 var(--teal-primary);
}

:deep(.el-tabs--card > .el-tabs__header .el-tabs__item:hover) {
  color: var(--teal-hover);
}

.content-area {
  flex: 1;
  overflow-y: auto;
  background-color: var(--surface-bg);
}
</style>

<style>
/* 全局搜索弹窗样式 */
.global-search-popper .el-autocomplete-suggestion__wrap {
  max-height: 400px;
}
.global-search-popper li {
  padding: 6px 12px;
}
.global-search-popper .search-item {
  padding: 4px 0;
}
.global-search-popper .search-item-content {
  display: flex;
  flex-direction: column;
  line-height: 1.2;
}
.global-search-popper .search-item-title {
  font-size: 14px;
  font-weight: 500;
  color: var(--text-primary);
  margin-bottom: 4px;
}
.global-search-popper .search-item-desc {
  font-size: 12px;
  color: var(--text-muted);
}
</style>
