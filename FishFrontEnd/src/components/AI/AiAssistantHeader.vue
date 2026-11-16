<template>
  <div class="chat-header">
    <div class="header-copy">
      <p class="header-eyebrow">AI Assistant</p>
      <h2>{{ title }}</h2>
    </div>
    <div class="header-meta">
      <span>{{ conversationCount }} 条会话</span>
    </div>
  </div>
</template>

<script setup>
defineProps({
  title: {
    type: String,
    default: '会话标题',
  },
  conversationCount: {
    type: Number,
    default: 0,
  },
})
</script>

<style scoped>
.chat-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 20px 24px;
  border-bottom: 1px solid var(--border-light);
  background:
    linear-gradient(135deg, rgba(8, 145, 178, 0.08), transparent 55%),
    linear-gradient(180deg, rgba(255, 255, 255, 0.95), rgba(248, 251, 252, 0.92));
}

.header-copy {
  min-width: 0;
}

.header-eyebrow {
  margin: 0 0 6px;
  color: var(--teal-primary);
  font-size: 12px;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
}

.chat-header h2 {
  margin: 0;
  color: var(--text-primary);
  font-size: 20px;
  font-weight: 700;
}

.header-meta {
  padding: 8px 12px;
  border: 1px solid rgba(8, 145, 178, 0.15);
  border-radius: 999px;
  color: var(--text-secondary);
  font-size: 12px;
  background-color: rgba(255, 255, 255, 0.75);
}
</style>
