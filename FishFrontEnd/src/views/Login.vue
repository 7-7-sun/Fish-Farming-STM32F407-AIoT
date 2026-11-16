<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { View, Hide } from '@element-plus/icons-vue'
import { ElMessage } from 'element-plus'
import AuthLeft from '../components/AuthLeft.vue'
import { login } from '../api/modules/user'
import { useUserStore } from '../store/user'

const router = useRouter()
const userStore = useUserStore()

const loginForm = ref({
  identifier: '',
  password: '',
})

const isSubmitting = ref(false)
const showPassword = ref(false)

const resolveToken = (responseData) => {
  // 直接返回 token 字段
  return responseData?.token || ''
}

const setToken = (token) => {
  // “保持登录”用 localStorage；否则用 sessionStorage（随浏览器会话结束而清除）
  sessionStorage.setItem('token', token)
  localStorage.removeItem('token')
}

const handleLogin = async () => {
  if (isSubmitting.value) return
  isSubmitting.value = true

  try {
    // 获取用户名和密码
    const username = String(loginForm.value.identifier || '').trim()
    const password = String(loginForm.value.password || '')

    // 简单验证
    if (!username) {
      ElMessage.warning('请输入用户名')
      return
    }
    if (!password) {
      ElMessage.warning('请输入密码')
      return
    }

    // 构建符合后端要求的参数
    const payload = {
      username, // 注意：字段名必须是 username
      password, // 字段名必须是 password
    }

    const res = await login(payload)
    const token = resolveToken(res)

    if (!token) {
      throw new Error('登录成功但未返回 token')
    }

    setToken(token)
    if (res.userInfo) {
      userStore.setUserInfo(res.userInfo)
    }
    ElMessage.success('登录成功')

    // 跳转到监控页面
    await router.push('/monitoring')
  } catch (error) {
    console.error('登录失败:', error)
    // 接口层已统一处理错误，这里只处理 token 缺失的特殊情况
    if (error?.message === '登录成功但未返回 token') {
      ElMessage.error(error.message)
    }
  } finally {
    isSubmitting.value = false
  }
}

const goToRegister = () => {
  router.push('/register')
}
</script>

<template>
  <div class="auth-wrapper">
    <!-- 左侧展示区 -->
    <AuthLeft />

    <!-- 右侧操作区 -->
    <div class="auth-right">
      <div class="form-container">
        <div class="form-header">
          <h2>欢迎回来</h2>
          <p>请登录您的系统管理面板</p>
        </div>

        <!-- 登录/注册 切换选项 -->
        <div class="auth-tabs">
          <button class="tab-btn active">登录</button>
          <button
            class="tab-btn"
            @click="goToRegister">
            创建账户
          </button>
        </div>

        <form
          @submit.prevent="handleLogin"
          class="login-form">
          <!-- 账号输入 -->
          <div class="input-group">
            <label>用户名</label>
            <div class="input-wrapper">
              <input
                type="text"
                v-model="loginForm.identifier"
                placeholder="UserName"
                required />
            </div>
          </div>

          <!-- 密码输入 -->
          <div class="input-group">
            <div class="label-row">
              <label>密码</label>
            </div>
            <div class="input-wrapper">
              <input
                :type="showPassword ? 'text' : 'password'"
                v-model="loginForm.password"
                placeholder="Password"
                required />
              <button
                type="button"
                class="password-toggle"
                @click="showPassword = !showPassword">
                <View v-if="!showPassword" class="view-icon" />
                <Hide v-else class="view-icon" />
              </button>
            </div>
          </div>

          <!-- 提交按钮 -->
          <button
            type="submit"
            class="submit-btn"
            :disabled="isSubmitting">
            {{ isSubmitting ? '登录中...' : '进入系统' }}
            <span class="arrow">→</span>
          </button>
        </form>
      </div>
    </div>
  </div>
</template>

<style scoped>
.auth-wrapper {
  display: flex;
  width: 100vw;
  height: 100vh;
  overflow: hidden;
}

.auth-right {
  flex: 1;
  background-color: var(--ocean-pale);
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  position: relative;
  padding: 40px;
}

.form-container {
  width: 100%;
  max-width: 440px;
}

.form-header h2 {
  font-size: 2.25rem;
  font-weight: 800;
  color: var(--ocean-deep);
  margin-bottom: 8px;
}

.form-header p {
  color: var(--text-secondary);
  margin-bottom: 32px;
}

.auth-tabs {
  display: flex;
  background: var(--ocean-light);
  padding: 4px;
  border-radius: 10px;
  margin-bottom: 32px;
}

.tab-btn {
  flex: 1;
  padding: 10px;
  border: none;
  background: transparent;
  font-weight: 600;
  color: var(--text-muted);
  cursor: pointer;
  border-radius: 8px;
  transition: all var(--transition);
}

.tab-btn.active {
  background: var(--surface-card);
  color: var(--teal-primary);
  box-shadow: var(--shadow-sm);
}

.input-group {
  margin-bottom: 24px;
}

.input-group label {
  display: block;
  font-size: 0.75rem;
  font-weight: 700;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
  margin-bottom: 8px;
}

.label-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.input-wrapper {
  position: relative;
  display: flex;
  align-items: center;
}

.input-wrapper input {
  width: 100%;
  padding: 14px 16px 14px 14px;
  background: var(--surface-card);
  border: 1px solid var(--border-light);
  border-radius: 10px;
  font-size: 1rem;
  transition: all var(--transition);
  color: var(--text-primary);
}

.input-wrapper input:focus {
  outline: none;
  border-color: var(--teal-primary);
  background: var(--surface-card);
  box-shadow: 0 0 0 4px var(--teal-glow);
}

.view-icon {
  width: 18px;
  height: 18px;
  color: var(--text-muted);
}

.password-toggle {
  position: absolute;
  right: 16px;
  background: none;
  border: none;
  color: var(--text-muted);
  cursor: pointer;
  display: flex;
  align-items: center;
}

.submit-btn {
  width: 100%;
  padding: 16px;
  background: linear-gradient(135deg, var(--ocean-mid), var(--ocean-dark));
  color: white;
  border: none;
  border-radius: 10px;
  font-size: 1rem;
  font-weight: 700;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;
  transition: all var(--transition);
}

.submit-btn:hover {
  background: linear-gradient(135deg, var(--ocean-surface), var(--ocean-mid));
  transform: translateY(-1px);
  box-shadow: var(--shadow-md);
}
</style>
