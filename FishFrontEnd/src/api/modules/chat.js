import request from '../request'

// 获取对话历史列表
export const getChatLists = () => {
  return request.get('/chat/history')
}

// 获取对话详情（根据对话ID）
export const getChat = (conversationId) => {
  return request.get(`/chat/history/${conversationId}`)
}

// 删除对话
export const deleteChat = (conversationId) => {
  return request.delete(`/chat/history/${conversationId}`)
}

// 发送消息
export const sendMessages = (data) => {
  return request.post('/chat/send', data)
}

