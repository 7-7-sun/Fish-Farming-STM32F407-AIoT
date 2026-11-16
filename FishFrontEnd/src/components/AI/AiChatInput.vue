<template>
  <div class="chat-input">
    <div class="input-shell">
      <el-input
        :model-value="modelValue"
        type="textarea"
        :rows="3"
        placeholder="请输入您的问题或指令..."
        resize="none"
        @update:model-value="emit('update:modelValue', $event)"
        @keydown.enter.prevent="emit('send')" />
      <div class="input-actions">
        <el-button
          v-if="isSending"
          type="danger"
          plain
          @click="emit('stop')">
          <el-icon><VideoPause /></el-icon>
          停止生成
        </el-button>
        <el-button @click="emit('clear')">清空对话</el-button>
        <el-button
          type="primary"
          :disabled="!modelValue.trim() && !isSending"
          :loading="isSending"
          @click="emit('send')">
          发送
        </el-button>
      </div>
    </div>
  </div>
</template>

<script setup>
import { VideoPause } from '@element-plus/icons-vue'

defineProps({
  modelValue: {
    type: String,
    default: '',
  },
  isSending: {
    type: Boolean,
    default: false,
  },
})

const emit = defineEmits(['update:modelValue', 'send', 'stop', 'clear'])
</script>

<style scoped>
.chat-input {
  padding: 18px 20px 20px;
  border-top: 1px solid var(--border-light);
  background:
    linear-gradient(180deg, rgba(255, 255, 255, 0.92), rgba(248, 251, 252, 0.98));
}

.input-shell {
  display: flex;
  flex-direction: column;
  gap: 12px;
  padding: 14px;
  border: 1px solid rgba(8, 145, 178, 0.12);
  border-radius: 18px;
  background-color: rgba(255, 255, 255, 0.86);
  box-shadow: 0 12px 28px rgba(15, 23, 42, 0.05);
}

.input-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
}
</style>
