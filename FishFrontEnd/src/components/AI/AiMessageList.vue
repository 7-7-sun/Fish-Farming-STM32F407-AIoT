<template>
  <div
    ref="messagesContainer"
    class="chat-messages">
    <div
      v-for="message in messages"
      :key="message.id"
      :class="['message-item', message.type]">
      <div class="message-content">
        <div class="message-avatar">
          <el-avatar
            :size="40"
            :icon="message.type === 'user' ? User : Platform" />
        </div>
        <div class="message-body">
          <div class="message-header">
            <span class="message-sender">{{ message.sender }}</span>
            <span class="message-time">{{ message.time }}</span>
          </div>
          <div
            class="message-text"
            v-html="formatMessage(message.content)"></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { User, Platform } from '@element-plus/icons-vue'

defineProps({
  messages: {
    type: Array,
    default: () => [],
  },
  formatMessage: {
    type: Function,
    required: true,
  },
})

const messagesContainer = ref(null)

defineExpose({
  messagesContainer,
})
</script>

<style scoped>
.chat-messages {
  height: calc(100vh - 280px);
  overflow-y: auto;
  padding: 24px 20px;
  background:
    radial-gradient(circle at top right, rgba(8, 145, 178, 0.08), transparent 30%),
    linear-gradient(180deg, rgba(243, 248, 250, 0.95), rgba(235, 244, 247, 0.98));
}

.message-item {
  margin-bottom: 20px;
}

.message-item.user {
  display: flex;
  justify-content: flex-end;
}

.message-item.ai {
  display: flex;
  justify-content: flex-start;
}

.message-content {
  display: flex;
  align-items: flex-start;
  max-width: 82%;
}

.message-item.user .message-content {
  flex-direction: row-reverse;
}

.message-avatar {
  margin: 0 12px;
}

.message-body {
  max-width: 100%;
  padding: 14px 16px;
  border: 1px solid var(--border-subtle);
  border-radius: 18px;
  background-color: rgba(255, 255, 255, 0.92);
  box-shadow: 0 10px 30px rgba(15, 23, 42, 0.06);
  backdrop-filter: blur(10px);
}

.message-item.user .message-body {
  border: none;
  color: white;
  background: linear-gradient(135deg, var(--teal-primary), var(--ocean-mid));
}

.message-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
  margin-bottom: 8px;
}

.message-sender {
  font-size: 14px;
  font-weight: 600;
}

.message-time {
  font-size: 12px;
  opacity: 0.72;
}

.message-item.user .message-time {
  color: rgba(255, 255, 255, 0.82);
}

.message-text {
  line-height: 1.7;
  word-wrap: break-word;
}

.chat-messages::-webkit-scrollbar {
  width: 6px;
}

.chat-messages::-webkit-scrollbar-track {
  background: #f1f1f1;
  border-radius: 3px;
}

.chat-messages::-webkit-scrollbar-thumb {
  background: #c1c1c1;
  border-radius: 3px;
}

.chat-messages::-webkit-scrollbar-thumb:hover {
  background: #a8a8a8;
}
</style>
