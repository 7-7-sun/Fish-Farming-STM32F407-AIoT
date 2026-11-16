import axios from 'axios'
import { ElMessage } from 'element-plus'
import router from '../router/index.js'
import.meta.env.VITE_API_BASE_URL

// 创建 axios 实例
const envBaseURL = import.meta.env.VITE_API_BASE_URL
const resolvedBaseURL = 'http://139.9.0.201:9000/api'
  // typeof envBaseURL === 'string' && envBaseURL.trim()
  //   ? envBaseURL.trim()
  //   : import.meta.env.DEV
  //     ? '/api'
  //     : 'http://localhost:5000/api'

const request = axios.create({
  baseURL: resolvedBaseURL,
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
})

// 请求拦截器
request.interceptors.request.use(
  (config) => {
    // 从 localStorage/sessionStorage 获取 token（配合“保持登录”功能）
    const token = localStorage.getItem('token') || sessionStorage.getItem('token')
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  (error) => {
    return Promise.reject(error)
  }
)

// 响应拦截器
request.interceptors.response.use(
  (response) => {
    const data = response.data

    if (data && typeof data === 'object' && 'success' in data) {
      if (data.success === true) {
        return data
      }
      const errMsg = data.message || data.response || '请求失败'
      ElMessage.error(errMsg)
      return Promise.reject(new Error(errMsg))
    }

    return data
  },
  (error) => {
    // HTTP 错误
    if (error.response) {
      switch (error.response.status) {
        case 401:
          ElMessage.error('未授权，请重新登录')
          localStorage.removeItem('token')
          sessionStorage.removeItem('token')
          router.push('/login')
          break
        case 403:
          ElMessage.error('拒绝访问')
          break
        case 404:
          ElMessage.error('请求的资源不存在')
          break
        case 500:
          ElMessage.error('服务器内部错误')
          break
        default:
          ElMessage.error(error.response.data.message || '请求失败')
      }
    } else if (error.request) {
      ElMessage.error('网络错误，请检查网络连接')
    } else {
      ElMessage.error('请求配置错误')
    }
    return Promise.reject(error)
  }
)

export default request