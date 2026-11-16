<template>
  <div class="ai-chat-page">
    <el-container class="page-container">
      <el-main class="chat-main">
        <AiAssistantHeader
          :title="currentConversationTitle"
          :conversation-count="conversations.length" />
        <AiMessageList
          ref="messageListRef"
          :messages="chatMessages"
          :format-message="formatMessage" />
        <AiChatInput
          v-model="inputMessage"
          :is-sending="isSending"
          @send="handleSendMessage"
          @stop="stopGenerating"
          @clear="clearChat" />
      </el-main>

      <AiConversationSidebar
        :conversations="conversations"
        :current-conversation-id="currentConversationId"
        :is-manage-mode="isManageMode"
        :selected-conversations="selectedConversations"
        :is-all-selected="isAllSelected"
        :format-date="formatDate"
        @create="createNewChat"
        @update:is-manage-mode="isManageMode = $event"
        @update:selected-conversations="selectedConversations = $event"
        @select-all="handleSelectAll"
        @exit-manage="exitManageMode"
        @conversation-click="handleConversationClick"
        @delete-chat="handleDeleteChat"
        @batch-delete="handleBatchDelete" />
    </el-container>
  </div>
</template>

<script setup>
import { ref, nextTick, onMounted, computed } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import { getChatLists, getChat, deleteChat } from '../api/modules/chat'
import MarkdownIt from 'markdown-it'
import AiAssistantHeader from '../components/AI/AiAssistantHeader.vue'
import AiMessageList from '../components/AI/AiMessageList.vue'
import AiChatInput from '../components/AI/AiChatInput.vue'
import AiConversationSidebar from '../components/AI/AiConversationSidebar.vue'

const md = new MarkdownIt({
  breaks: true, // 允许回车换行
  linkify: true // 自动识别链接
})

// 响应式数据
const inputMessage = ref('')
const isSending = ref(false)
const messageListRef = ref(null)
const currentConversationId = ref(0)
const abortController = ref(null)

// 模拟聊天消息数据
const chatMessages = ref([])

// 临时缓存：保存刚创建的对话消息，避免切换回来变空白
const tempMessageCache = ref(new Map())

// 模拟历史对话数据
const conversations = ref([])

const normalizeConversation = (conversation = {}, index = 0, previousConversation = null) => {
  const resolvedId =
    conversation.id ??
    conversation.conversationId ??
    conversation.uuid ??
    previousConversation?.id ??
    `mock_id_${index}`

  const preview =
    conversation.preview ||
    conversation.lastMessage ||
    conversation.content ||
    conversation.firstMessage ||
    conversation.summary ||
    previousConversation?.lastMessage ||
    (typeof conversation.messageCount === 'number' ? `共 ${conversation.messageCount} 条消息` : '...')

  const updateSource =
    conversation.updatedAt ||
    conversation.updateTime ||
    conversation.updatedTime ||
    conversation.createdAt ||
    conversation.createTime ||
    previousConversation?.updateTime ||
    Date.now()

  const parsedUpdateTime = new Date(updateSource)

  return {
    id: resolvedId,
    title: conversation.title || previousConversation?.title || '对话记录',
    lastMessage: preview,
    updateTime: isNaN(parsedUpdateTime.getTime()) ? new Date() : parsedUpdateTime,
    unreadCount: conversation.unreadCount || previousConversation?.unreadCount || 0,
  }
}

const fetchConversationList = async ({ keepDraft = false } = {}) => {
  const list = await getChatLists()
  const listData = Array.isArray(list) ? list : (list?.conversations || list?.data || [])
  const previousConversationMap = new Map(
    conversations.value.map((conversation) => [String(conversation.id), conversation])
  )
  const normalizedList = listData.map((conversation, index) => {
    const resolvedId = conversation.id ?? conversation.conversationId ?? conversation.uuid ?? `mock_id_${index}`
    const previousConversation = previousConversationMap.get(String(resolvedId))
    return normalizeConversation(conversation, index, previousConversation)
  })

  const draftConversation = keepDraft
    ? conversations.value.find((conversation) => Number(conversation.id) === 0)
    : null

  conversations.value = draftConversation
    ? [draftConversation, ...normalizedList.filter((conversation) => Number(conversation.id) !== 0)]
    : normalizedList

  return conversations.value
}

// 批量管理状态
const isManageMode = ref(false)
const selectedConversations = ref([])
const currentConversationTitle = computed(() => {
  if (Number(currentConversationId.value) === 0) {
    return '新建对话'
  }

  const currentConversation = conversations.value.find(
    (conversation) => String(conversation.id) === String(currentConversationId.value)
  )

  return currentConversation?.title || '会话标题'
})

const isAllSelected = computed({
  get: () => {
    // 排除掉新建未保存的对话（id为0的）
    const validConversations = conversations.value.filter(c => Number(c.id) !== 0)
    return validConversations.length > 0 && selectedConversations.value.length === validConversations.length
  },
  set: (val) => {
    if (val) {
      selectedConversations.value = conversations.value.filter(c => Number(c.id) !== 0).map(c => c.id)
    } else {
      selectedConversations.value = []
    }
  }
})

const handleSelectAll = (val) => {
  isAllSelected.value = val
}

const exitManageMode = () => {
  isManageMode.value = false
  selectedConversations.value = []
}

const handleConversationClick = (id) => {
  if (isManageMode.value) {
    if (Number(id) === 0) return // 新建的对话不可选中
    const index = selectedConversations.value.indexOf(id)
    if (index > -1) {
      selectedConversations.value.splice(index, 1)
    } else {
      selectedConversations.value.push(id)
    }
  } else {
    loadConversation(id)
  }
}

// 批量删除
const handleBatchDelete = () => {
  if (selectedConversations.value.length === 0) return
  
  ElMessageBox.confirm(`确定要删除选中的 ${selectedConversations.value.length} 个对话吗？`, '批量删除确认', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    type: 'warning',
  }).then(async () => {
    const idsToDelete = [...selectedConversations.value]
    
    // 乐观删除
    conversations.value = conversations.value.filter((c) => !idsToDelete.includes(c.id))
    
    // 如果当前正在查看的对话被删除了
    if (idsToDelete.includes(currentConversationId.value)) {
      if (conversations.value.length > 0) {
        loadConversation(conversations.value[0].id)
      } else {
        createNewChat()
      }
    }

    exitManageMode()

    try {
      await Promise.all(idsToDelete.filter(id => Number(id) !== 0).map(id => deleteChat(id)))
      ElMessage.success('批量删除成功')
    } catch (error) {
      console.warn('后端批量删除接口部分异常，前端已自动完成删除动作:', error)
    }
  }).catch(() => {})
}

// 滚动到底部
const scrollToBottom = () => {
  nextTick(() => {
    const scrollContainer =
      messageListRef.value?.messagesContainer?.value ?? messageListRef.value?.messagesContainer

    if (scrollContainer) {
      scrollContainer.scrollTop = scrollContainer.scrollHeight
    }
  })
}

// 格式化消息内容 (Markdown 渲染)
const formatMessage = (content) => {
  if (!content) return ''
  
  // 1. 深度清洗后端可能带来的变态格式
  // 有些流直接吐出了类似 `"您好" \n\n "！" \n\n` 的东西，先把它们整体清洗一遍
  // 移除所有首尾的空格和换行
  let cleanStr = content.trim()
  
  // 去除包裹在每个词外面的双引号 (比如 "您好" -> 您好)
  // 全局替换：匹配所有 "xxx" 并提取里面的内容，同时保留可能原本就需要的引号
  cleanStr = cleanStr.replace(/"(.*?)"/g, (match, p1) => {
    // 如果匹配到的是类似 "{"conversationId": 8}" 这种 json，就不管它
    if (p1.includes('{') || p1.includes('}')) return match
    // 否则把双引号脱掉
    return p1
  })
  
  // 清理流里面可能存在的单独残留双引号
  cleanStr = cleanStr.replace(/^"+|"+$/g, '')
  cleanStr = cleanStr.replace(/ "\n/g, '\n').replace(/\n" /g, '\n')
  
  // 2. 处理换行
  // 后端可能直接把换行符序列化成了 \n 字面量，转回真实的换行
  cleanStr = cleanStr.replace(/\\n/g, '\n')
  // 针对类似 "您好" \n "！" 这种被截断的奇怪空行，把多余空行压缩一下
  cleanStr = cleanStr.replace(/\n\s*\n\s*\n/g, '\n\n')
  
  // 处理未转码的 Unicode Emoji (比如 \uD83C\uDFAF -> 🎯)
  cleanStr = cleanStr.replace(/\\u([0-9a-fA-F]{4})/g, (_, grp) => {
    return String.fromCharCode(parseInt(grp, 16))
  })
  
  // 强力剔除所有可能出现的 conversationId JSON 字符串（不管它在哪里、有没有引号）
  cleanStr = cleanStr.replace(/\{"conversationId":\s*\d+\}/g, '')
  cleanStr = cleanStr.replace(/"\{"conversationId":\s*\d+\}"/g, '')
  cleanStr = cleanStr.replace(/\\?\"?\{"conversationId":\s*\d+\}\\?\"?/g, '')
  
  // 3. 使用 markdown-it 进行渲染
  return md.render(cleanStr)
}

// 格式化日期
const formatDate = (date) => {
  if (!date || isNaN(new Date(date).getTime())) return ''
  return new Date(date).toLocaleString('zh-CN', {
    month: 'short',
    day: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
  })
}

// 停止生成
const stopGenerating = () => {
  if (abortController.value) {
    abortController.value.abort()
    abortController.value = null
    isSending.value = false
    ElMessage.info('已停止生成')
  }
}

// 发送消息
const handleSendMessage = async () => {
  if (isSending.value) return
  if (!inputMessage.value.trim()) return

  const userMessage = {
    id: Date.now(),
    type: 'user',
    sender: '用户',
    time: new Date().toLocaleTimeString('zh-CN', {
      hour: '2-digit',
      minute: '2-digit',
    }),
    content: inputMessage.value,
  }

  chatMessages.value.push(userMessage)
  const messageText = inputMessage.value
  inputMessage.value = ''
  isSending.value = true
  scrollToBottom()

  // 提前创建一个空的 AI 回复占位，后续用于流式拼接文字
  const aiResponse = {
    id: Date.now() + 1,
    type: 'ai',
    sender: 'Submerged AI',
    time: new Date().toLocaleTimeString('zh-CN', {
      hour: '2-digit',
      minute: '2-digit',
    }),
    content: '',
  }
  chatMessages.value.push(aiResponse)
  const aiMessageRef = chatMessages.value[chatMessages.value.length - 1]

  try {
    const token = localStorage.getItem('token') || sessionStorage.getItem('token')
    const isDraftConversation = Number(currentConversationId.value) === 0
    
    // 构造 SSE 流式请求 URL
    const baseURL = import.meta.env.DEV ? '/api' : (import.meta.env.VITE_API_BASE_URL || '/api')
    let url = `${baseURL}/chat/stream?message=${encodeURIComponent(messageText)}`
    if (!isDraftConversation) {
      url += `&conversationId=${currentConversationId.value}`
    } else {
      url += '&forceNew=true'
    }

    abortController.value = new AbortController()

    const response = await fetch(url, {
      method: 'GET',
      headers: {
        'Authorization': token ? `Bearer ${token}` : '',
        'Accept': 'text/event-stream'
      },
      signal: abortController.value.signal
    })

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`)
    }

    // 监听和解析数据流 (打字机效果)
    const reader = response.body.getReader()
    const decoder = new TextDecoder('utf-8')
    let buffer = ''
    let isStreamFinished = false

    while (!isStreamFinished) {
      const { done, value } = await reader.read()
      if (done) break
      
      const chunkStr = decoder.decode(value, { stream: true })
      buffer += chunkStr
      
      let i = 0;
      while (i < buffer.length) {
        const remaining = buffer.slice(i);
        
        if (remaining.startsWith('event:message')) {
          i += 13;
          while (i < buffer.length && (buffer[i] === ' ' || buffer[i] === '\n' || buffer[i] === '\r')) {
            i++;
          }
        } else if (remaining.startsWith('event:done')) {
          i += 10;
        } else if (remaining.startsWith('event:error')) {
          i += 11;
          isStreamFinished = true;
          break;
        } else if (remaining.startsWith('event:complete')) {
          // 处理后端返回的完整对话ID信息
          const completeMatch = remaining.match(/^event:complete[\s\S]*?({.*?})/);
          if (completeMatch) {
            try {
              const data = JSON.parse(completeMatch[1]);
              const newId = data.conversationId;
              if (newId && currentConversationId.value === 0) {
                const existingConv = conversations.value.find(c => String(c.id) === String(newId));
                if (existingConv) {
                  // 后端复用了已有的会话（5分钟内），移除前端新建的占位会话，避免重复 key 漏洞
                  conversations.value = conversations.value.filter(c => Number(c.id) !== 0);
                  currentConversationId.value = newId;
                  existingConv.updateTime = new Date();
                  existingConv.lastMessage = messageText.substring(0, 20) + '...';
                  
                  // 提示用户并拉取该会话历史记录
                  ElMessage.info('五分钟内已有活跃对话，已自动为您继续上一次会话');
                  getChat(newId).then(response => {
                    const history = Array.isArray(response) ? response : (response?.messages || response?.data || []);
                    const historyMapped = history.map((msg) => ({
                      id: msg.id || Date.now() + Math.random(),
                      type: msg.role === 1 ? 'user' : 'ai',
                      sender: msg.role === 1 ? '用户' : 'Submerged AI',
                      time: msg.createdAt ? new Date(msg.createdAt).toLocaleTimeString('zh-CN', {
                        hour: '2-digit',
                        minute: '2-digit',
                      }) : '',
                      content: msg.content,
                    }));
                    if (chatMessages.value.length === 2) {
                      chatMessages.value = [...historyMapped, ...chatMessages.value];
                      nextTick(() => scrollToBottom());
                    }
                  }).catch(e => console.error('拉取被复用会话的历史记录失败:', e));
                } else {
                  // 真正的全新会话
                  currentConversationId.value = newId;
                  const newConv = conversations.value.find(c => Number(c.id) === 0);
                  if (newConv) {
                    newConv.id = newId;
                    newConv.title = messageText.substring(0, 10) + '...';
                  }
                }
              }
            } catch(e){}
            i += completeMatch[0].length;
            isStreamFinished = true;
            break;
          } else if (remaining.includes('{') && !remaining.includes('}')) {
            break; // 正在接收 JSON，等待下个片段
          } else if (remaining === 'event:complete' || remaining === 'event:complete\n' || remaining === 'event:complete\ndata:') {
            break; // 等待 JSON 开始
          } else {
            i += 14; // 格式不符，跳过
          }
        } else {
          // 匹配并跳过无关标记
          const idMatch = remaining.match(/^id:\d+/);
          if (idMatch) {
            i += idMatch[0].length;
            continue;
          }
          
          const dataMatch = remaining.match(/^data:\s*/);
          if (dataMatch) {
            i += dataMatch[0].length;
            continue;
          }
          
          const doneMatch = remaining.match(/^\[DONE\]/);
          if (doneMatch) {
            i += 6;
            continue;
          }
          
          // 检查是否是被截断的标记
          let isPartialMarker = false;
          const markers = ['event:message', 'event:done', 'event:error', 'event:complete', 'data:', 'id:', '[DONE]'];
          for (const m of markers) {
            if (m.startsWith(remaining)) {
              isPartialMarker = true;
              break;
            }
          }
          
          if (isPartialMarker) {
            break; // 截断了，等待下个数据流片段
          }
          
          // 真正的纯净文字内容，拼接到页面
          let char = buffer[i];
          if (char !== '\n' && char !== '\r') {
            chatMessages.value[chatMessages.value.length - 1].content += char;
          }
          i++;
        }
      }
      buffer = buffer.slice(i);
      
      // 实时清理在 buffer 流中积累出来的 {"conversationId": 8} 这种残留
      const currentContent = chatMessages.value[chatMessages.value.length - 1].content;
      
      // 暴力截断：只要发现出现了 "{"conversationId" 这个开头，直接把后面的所有内容全部切掉
      const convIndex = currentContent.indexOf('{"conversationId"');
      const convIndexWithQuote = currentContent.indexOf('"{"conversationId"');
      
      if (convIndex !== -1 || convIndexWithQuote !== -1) {
        const cutIndex = convIndexWithQuote !== -1 ? convIndexWithQuote : convIndex;
        chatMessages.value[chatMessages.value.length - 1].content = currentContent.substring(0, cutIndex);
        
        // 尝试从中提取ID
        try {
          const jsonStrMatch = currentContent.substring(cutIndex).match(/({.*"conversationId".*?})/);
          if (jsonStrMatch) {
            const data = JSON.parse(jsonStrMatch[1].replace(/^"/, '').replace(/"$/, ''));
            const newId = data.conversationId;
            if (newId && currentConversationId.value === 0) {
              const existingConv = conversations.value.find(c => String(c.id) === String(newId));
              if (existingConv) {
                // 后端复用了已有的会话（5分钟内），移除前端新建的占位会话，避免重复 key 漏洞
                conversations.value = conversations.value.filter(c => Number(c.id) !== 0);
                currentConversationId.value = newId;
                existingConv.updateTime = new Date();
                existingConv.lastMessage = messageText.substring(0, 20) + '...';
                
                // 提示用户并拉取该会话历史记录
                ElMessage.info('五分钟内已有活跃对话，已自动为您继续上一次会话');
                getChat(newId).then(response => {
                  const history = Array.isArray(response) ? response : (response?.messages || response?.data || []);
                  const historyMapped = history.map((msg) => ({
                    id: msg.id || Date.now() + Math.random(),
                    type: msg.role === 1 ? 'user' : 'ai',
                    sender: msg.role === 1 ? '用户' : 'Submerged AI',
                    time: msg.createdAt ? new Date(msg.createdAt).toLocaleTimeString('zh-CN', {
                      hour: '2-digit',
                      minute: '2-digit',
                    }) : '',
                    content: msg.content,
                  }));
                  if (chatMessages.value.length === 2) {
                    chatMessages.value = [...historyMapped, ...chatMessages.value];
                    nextTick(() => scrollToBottom());
                  }
                }).catch(e => console.error('拉取被复用会话的历史记录失败:', e));
              } else {
                // 真正的全新会话
                currentConversationId.value = newId;
                const newConv = conversations.value.find(c => Number(c.id) === 0);
                if (newConv) {
                  newConv.id = newId;
                  newConv.title = messageText.substring(0, 10) + '...';
                }
              }
            }
          }
        } catch(e) {}
        
        // 既然已经收到了带有 conversationId 的 JSON 残留，说明流一定已经结束了
        isStreamFinished = true;
      }
      
      scrollToBottom()
    }

    // 消息全部接收完毕，更新左侧列表的最后一条消息预览
    const curConv = conversations.value.find(c => String(c.id) === String(currentConversationId.value))
    if (curConv) {
      curConv.lastMessage = aiMessageRef.content.substring(0, 20) + '...'
      curConv.updateTime = new Date()
    }

    // 保存到临时缓存，避免刚创建的对话切换回来变空白
    if (currentConversationId.value !== 0) {
      tempMessageCache.value.set(String(currentConversationId.value), [...chatMessages.value])
    }

    // 以服务端返回的会话列表为准重新同步，避免新建对话只存在于当前页面内存中
    await fetchConversationList()

  } catch (e) {
    if (e.name === 'AbortError') {
      aiMessageRef.content += '\n（已停止生成）'
    } else {
      console.error('流式响应出错:', e)
      aiMessageRef.content += '\n（请求失败或连接中断）'
    }
  } finally {
    isSending.value = false
    abortController.value = null
    scrollToBottom()
  }
}

// 加载历史对话
const loadConversation = async (conversationId) => {
  currentConversationId.value = conversationId
  chatMessages.value = []

  if (Number(conversationId) === 0) {
    return // 新建的空对话不需要请求接口
  }

  // 首先检查临时缓存，如果有就直接使用（避免刚创建的对话切换回来变空白）
  const cachedMessages = tempMessageCache.value.get(String(conversationId))
  if (cachedMessages && cachedMessages.length > 0) {
    chatMessages.value = [...cachedMessages]
    nextTick(() => scrollToBottom())
  }

  try {
    const response = await getChat(conversationId)
    const messages = Array.isArray(response) ? response : (response?.messages || response?.data || [])
    
    if (messages && messages.length > 0) {
      // role: 1 = user, 0 = ai (根据后端逻辑)
      chatMessages.value = messages.map((msg) => ({
        id: msg.id || Date.now() + Math.random(),
        type: msg.role === 1 ? 'user' : 'ai',
        sender: msg.role === 1 ? '用户' : 'Submerged AI',
        time: msg.createdAt ? new Date(msg.createdAt).toLocaleTimeString('zh-CN', {
          hour: '2-digit',
          minute: '2-digit',
        }) : '',
        content: msg.content,
      }))
      
      // 更新缓存
      tempMessageCache.value.set(String(conversationId), [...chatMessages.value])
    } else if (!cachedMessages || cachedMessages.length === 0) {
      // 接口返回空且没有缓存时，显示提示
      console.warn('加载对话消息为空，conversationId:', conversationId)
    }
    
    scrollToBottom()
  } catch (e) {
    // 错误由拦截器处理 
    console.error('加载对话失败:', e)
  }
}

// 创建新对话
const createNewChat = () => {
  // 避免重复添加新对话
  if (conversations.value.some(c => Number(c.id) === 0)) {
    currentConversationId.value = 0
    chatMessages.value = []
    ElMessage.info('当前已经是新对话了，请直接在下方输入内容')
    return
  }

  const newConversation = {
    id: 0, // 新对话ID统一用0，发送时后端会分配真实ID
    title: '新对话',
    lastMessage: '在下方输入消息开始对话...',
    updateTime: new Date(),
    unreadCount: 0,
  }

  conversations.value.unshift(newConversation)
  chatMessages.value = []
  currentConversationId.value = 0

  ElMessage.success('新对话已创建')
}

// 删除对话
const handleDeleteChat = (id) => {
  if (Number(id) === 0) {
    // 尚未保存的新对话直接移除
    conversations.value = conversations.value.filter((c) => Number(c.id) !== 0)
    if (Number(currentConversationId.value) === 0) {
      if (conversations.value.length > 0) {
        loadConversation(conversations.value[0].id)
      } else {
        createNewChat()
      }
    }
    return
  }

  ElMessageBox.confirm('确定要删除该对话记录吗？', '删除确认', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    type: 'warning',
  })
    .then(async () => {
      // 乐观删除：为了防止后端接口报错(比如500)导致页面卡住，前端先直接把它从列表中移除
      conversations.value = conversations.value.filter((c) => String(c.id) !== String(id))
      
      // 如果删除的是当前正在查看的对话，自动加载下一条，或者新建一个空对话
      if (String(currentConversationId.value) === String(id)) {
        if (conversations.value.length > 0) {
          loadConversation(conversations.value[0].id)
        } else {
          createNewChat()
        }
      }

      try {
        await deleteChat(id)
        ElMessage.success('对话已删除')
      } catch (error) {
        // 如果后端报错，由于前端已经乐观删除了，不影响用户体验，控制台打印一下即可
        console.warn('后端删除接口异常，前端已自动完成删除动作:', error)
      }
    })
    .catch(() => {})
}

// 清空当前对话消息
const clearChat = () => {
  ElMessageBox.confirm('确定要清空当前对话吗？', '确认清空', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    type: 'warning',
  })
    .then(() => {
      chatMessages.value = []
      ElMessage.success('对话已清空')
    })
    .catch(() => {
      ElMessage.info('已取消清空')
    })
}

// 组件挂载时加载对话列表
onMounted(async () => {
  try {
    await fetchConversationList()

    // 默认加载第一条对话
    if (conversations.value.length > 0) {
      await loadConversation(conversations.value[0].id)
    } else {
      createNewChat()
    }
  } catch (e) {
    // 错误由拦截器处理
    console.error('getChatLists error:', e)
  }
})
</script>

<style scoped>
.ai-chat-page {
  min-height: 100%;
  background-color: var(--surface-bg);
}

.page-container {
  min-height: calc(100vh - 84px);
}

.chat-main {
  display: flex;
  flex-direction: column;
  padding: 0;
  margin: 10px;
  overflow: hidden;
  border: 1px solid var(--border-light);
  border-radius: var(--radius-md);
  background-color: var(--surface-card);
  box-shadow: 0 18px 40px rgba(15, 23, 42, 0.06);
}

/* 覆盖 markdown-it 默认样式 */
:deep(.message-text p) {
  margin-top: 0;
  margin-bottom: 8px;
}
:deep(.message-text p:last-child) {
  margin-bottom: 0;
}
:deep(.message-text ul), :deep(.message-text ol) {
  margin-top: 4px;
  margin-bottom: 8px;
  padding-left: 20px;
}
:deep(.message-text li) {
  margin-bottom: 4px;
}
:deep(.message-text strong) {
  font-weight: 700;
  color: inherit;
}
:deep(.message-text h1), :deep(.message-text h2), :deep(.message-text h3), :deep(.message-text h4) {
  margin-top: 12px;
  margin-bottom: 8px;
  font-weight: 700;
}
:deep(.message-text hr) {
  border: 0;
  border-top: 1px solid var(--border-light);
  margin: 12px 0;
}
:deep(.message-text table) {
  border-collapse: collapse;
  width: 100%;
  margin-bottom: 12px;
}
:deep(.message-text th), :deep(.message-text td) {
  border: 1px solid var(--border-light);
  padding: 6px 12px;
  text-align: left;
}
:deep(.message-text th) {
  background-color: rgba(0, 0, 0, 0.05);
}
:deep(.message-item.user .message-text th) {
  background-color: rgba(255, 255, 255, 0.1);
}
:deep(.message-item.user .message-text th),
:deep(.message-item.user .message-text td),
:deep(.message-item.user .message-text hr) {
  border-color: rgba(255, 255, 255, 0.2);
}
</style>
