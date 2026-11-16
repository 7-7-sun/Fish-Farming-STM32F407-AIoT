import request from '../request'

// 用户登录
export const login = (data) => {
  return request.post('/user/login', data)
}

// 用户注册
export const register = (data) => {
  return request.post('/user/register', data)
}

// 验证token
export const tokenVerify = (token) => {
  return request.get('/user/verify', { 
      headers: {
        Authorization: token
      }
   })
}

// 获取用户信息
export const getUsers = (userId) => {
  return request.get('/user/info', { 
      params:{
        userId
      }
   })
}