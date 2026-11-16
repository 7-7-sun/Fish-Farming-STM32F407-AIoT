import { createRouter, createWebHistory } from 'vue-router'

// 导入页面组件
import Login from '../views/Login.vue'
import Register from '../views/Register.vue'
import Monitoring from '../views/Monitoring.vue'
import AiAssistant from '../views/AiAssistant.vue'
import DataAnalysis from '../views/DataAnalysis.vue'
import UserManagement from '../views/UserManagement.vue'

import FeedRecords from '../views/FeedRecords.vue'
import Weather from '../views/Weather.vue'
import AlertManagement from '../views/AlertManagement.vue'
import EnvironmentRanges from '../views/EnvironmentRanges.vue'
import FishAdvice from '../views/FishAdvice.vue'
import Layout from '../views/Layout.vue'

const routes = [
  {
    path: '/',
    redirect: '/monitoring'  // 默认重定向到监控页面
  },
  {
    path: '/login',
    name: 'Login',
    component: Login,
    meta: { requiresAuth: false }
  },
  {
    path: '/register',
    name: 'Register',
    component: Register,
    meta: { requiresAuth: false }
  },
  {
    path: '/layout',
    name: 'Layout',
    component: Layout,
    children: [
      {
        path: '/monitoring',
        name: 'Monitoring',
        component: Monitoring,
        meta: { title: '实时监控' }
      },
      {
        path: '/aiAssistant',
        name: 'AiAssistant',
        component: AiAssistant,
        meta: { title: 'AI助手' }
      },
      {
        path: '/dataAnalysis',
        name: 'DataAnalysis',
        component: DataAnalysis,
        meta: { title: '自动化控制' }
      },
      {
        path: '/userManagement',
        name: 'UserManagement',
        component: UserManagement,
        meta: { title: '用户管理' }
      },

      {
        path: '/feedRecords',
        name: 'FeedRecords',
        component: FeedRecords,
        meta: { title: '投喂记录' }
      },
      {
        path: '/weather',
        name: 'Weather',
        component: Weather,
        meta: { title: '实时天气' }
      },
      {
        path: '/alertManagement',
        name: 'AlertManagement',
        component: AlertManagement,
        meta: { title: '告警管理' }
      },
      {
        path: '/environmentRanges',
        name: 'EnvironmentRanges',
        component: EnvironmentRanges,
        meta: { title: '环境参数范围' }
      },
      {
        path: '/fishAdvice',
        name: 'FishAdvice',
        component: FishAdvice,
        meta: { title: '养鱼建议' }
      }
    ]
  }
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes
})

// 路由守卫：检查是否需要登录
router.beforeEach((to, from, next) => {
  const isAuthenticated = localStorage.getItem('token') || sessionStorage.getItem('token')  // 简单示例，实际应该用更安全的方式
  
  // 默认所有路由都需要认证，除非明确设置requiresAuth: false+
  if (to.meta.requiresAuth !== false && !isAuthenticated) {
    // 如果需要登录但未登录，跳转到登录页
    next('/login')
  } else {
    next()
  }
})

export default router