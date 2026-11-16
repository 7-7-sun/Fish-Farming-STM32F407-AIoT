<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage } from 'element-plus'
import AuthLeft from '../components/AuthLeft.vue'
import { register } from '../api/modules/user'

const router = useRouter()

const registerForm = ref({
  username: '',
  email: '',
  nickname: '',
  password: '',
})

const isSubmitting = ref(false)

const resolveToken = (responseData) => {
  return responseData?.token || ''
}

const setToken = (token) => {
  // 注册接口若直接返回 token，则默认写入 localStorage 以便刷新后仍保持登录态
  localStorage.setItem('token', token)
  sessionStorage.removeItem('token')
}

// 存储用户信息
const setUserInfo = (userInfo) => {
  if (userInfo && typeof userInfo === 'object') {
    localStorage.setItem('userInfo', JSON.stringify(userInfo))
  }
}

const handleRegister = async () => {
  if (isSubmitting.value) return

  // 表单验证
  const { username, email, nickname, password } = registerForm.value

  if (!username.trim()) {
    ElMessage.warning('请输入用户名')
    return
  }

  if (!email.trim()) {
    ElMessage.warning('请输入邮箱')
    return
  }

  if (!password) {
    ElMessage.warning('请输入密码')
    return
  }

  isSubmitting.value = true

  try {
    // 发送注册请求，包含所有必填字段
    const res = await register({
      username: username.trim(),
      email: email.trim(),
      nickname: nickname.trim() || username.trim(), // 如果没有填昵称，使用用户名
      password: password,
    })

    // 解析 token
    const token = resolveToken(res)

    if (token) {
      // 存储 token
      setToken(token)

      // 存储用户信息
      const userInfo = res?.userInfo
      if (userInfo) {
        setUserInfo(userInfo)
      }

      ElMessage.success('注册并登录成功')
      await router.push('/monitoring')
      return
    }

    // 如果注册成功但没有返回 token，跳转到登录页
    ElMessage.success('注册成功，请登录')
    await router.push('/login')
  } catch (error) {
    // 错误信息由接口拦截器统一处理
    console.error('注册失败:', error)
  } finally {
    isSubmitting.value = false
  }
}

const goToLogin = () => {
  router.push('/login')
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
          <h2>创建账户</h2>
          <p>开始您的智能水产养殖管理之旅</p>
        </div>

        <!-- 登录/注册 切换选项 -->
        <div class="auth-tabs">
          <button
            class="tab-btn"
            @click="goToLogin">
            登录
          </button>
          <button class="tab-btn active">创建账户</button>
        </div>

        <form
          @submit.prevent="handleRegister"
          class="register-form">
          <!-- 用户名 -->
          <div class="input-group">
            <label>用户名</label>
            <div class="input-wrapper">
              <input
                type="text"
                v-model="registerForm.username"
                placeholder="UserName"
                required />
            </div>
          </div>

          <!-- 昵称  -->
          <div class="input-group">
            <label>昵称</label>
            <div class="input-wrapper">
              <input
                type="text"
                v-model="registerForm.nickname"
                placeholder="NickName" />
            </div>
          </div>

          <!-- 邮箱 -->
          <div class="input-group">
            <label>系统邮箱</label>
            <div class="input-wrapper">
              <input
                type="email"
                v-model="registerForm.email"
                placeholder="Email"
                required />
            </div>
          </div>

          <!-- 密码 -->
          <div class="input-group">
            <label>密码</label>
            <div class="input-wrapper">
              <input
                type="password"
                v-model="registerForm.password"
                placeholder="Password"
                required />
            </div>
          </div>

          <!-- 提交按钮 -->
          <button
            type="submit"
            class="submit-btn"
            :disabled="isSubmitting">
            {{ isSubmitting ? '注册中...' : '创建账户' }}
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
  margin-bottom: 20px;
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

.input-wrapper {
  position: relative;
  display: flex;
  align-items: center;
}

.input-wrapper input {
  width: 100%;
  padding: 12px 16px 12px 14px;
  background: var(--surface-card);
  border: 1px solid var(--border-light);
  border-radius: 10px;
  font-size: 1rem;
  color: var(--text-primary);
  transition: all var(--transition);
}

.input-wrapper input:focus {
  outline: none;
  border-color: var(--teal-primary);
  background: var(--surface-card);
  box-shadow: 0 0 0 4px var(--teal-glow);
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
