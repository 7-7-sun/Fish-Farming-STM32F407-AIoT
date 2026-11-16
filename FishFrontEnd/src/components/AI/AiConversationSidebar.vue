<template>
  <el-aside
    width="320px"
    class="history-sidebar">
    <div class="sidebar-header">
      <div>
        <p class="sidebar-eyebrow">Memory</p>
        <h3>历史对话</h3>
      </div>
      <div class="header-actions">
        <template v-if="!isManageMode">
          <el-button
            type="primary"
            link
            @click="emit('create')">
            <el-icon><Plus /></el-icon>
            新建
          </el-button>
          <el-button
            type="primary"
            link
            @click="emit('update:isManageMode', true)">
            管理
          </el-button>
        </template>
        <template v-else>
          <el-checkbox
            :model-value="isAllSelected"
            @change="emit('select-all', $event)">
            全选
          </el-checkbox>
          <el-button
            type="primary"
            link
            @click="emit('exit-manage')">
            取消
          </el-button>
        </template>
      </div>
    </div>

    <div class="history-list">
      <div
        v-for="conversation in conversations"
        :key="conversation.id"
        :class="[
          'conversation-item',
          { active: String(currentConversationId) === String(conversation.id) && !isManageMode },
        ]"
        @click="emit('conversation-click', conversation.id)">
        <div class="conversation-content-wrapper">
          <el-checkbox
            v-if="isManageMode"
            :model-value="selectedConversations.includes(conversation.id)"
            :label="conversation.id"
            @click.stop
            @change="toggleSelection(conversation.id, $event)">
            <span class="hidden-label"></span>
          </el-checkbox>
          <div
            class="conversation-details"
            :class="{ 'conversation-details-manage': isManageMode }">
            <div class="conversation-title">
              {{ conversation.title || '对话记录' }}
            </div>
            <div class="conversation-info">
              <span class="conversation-time">{{ formatDate(conversation.updateTime) }}</span>
              <el-button
                v-if="!isManageMode"
                type="danger"
                link
                size="small"
                @click.stop="emit('delete-chat', conversation.id)">
                删除
              </el-button>
            </div>
            <div class="conversation-preview">
              {{ conversation.lastMessage || '...' }}
            </div>
          </div>
        </div>
      </div>
    </div>

    <div
      v-if="isManageMode"
      class="sidebar-footer">
      <el-button
        type="danger"
        :disabled="selectedConversations.length === 0"
        class="batch-delete-button"
        @click="emit('batch-delete')">
        删除选中的对话 ({{ selectedConversations.length }})
      </el-button>
    </div>
  </el-aside>
</template>

<script setup>
import { Plus } from '@element-plus/icons-vue'

const props = defineProps({
  conversations: {
    type: Array,
    default: () => [],
  },
  currentConversationId: {
    type: [String, Number],
    default: 0,
  },
  isManageMode: {
    type: Boolean,
    default: false,
  },
  selectedConversations: {
    type: Array,
    default: () => [],
  },
  isAllSelected: {
    type: Boolean,
    default: false,
  },
  formatDate: {
    type: Function,
    required: true,
  },
})

const emit = defineEmits([
  'create',
  'update:isManageMode',
  'select-all',
  'exit-manage',
  'conversation-click',
  'delete-chat',
  'batch-delete',
  'update:selectedConversations',
])

const toggleSelection = (conversationId, checked) => {
  if (Number(conversationId) === 0) return

  const nextSelected = checked
    ? [...props.selectedConversations, conversationId]
    : props.selectedConversations.filter((id) => String(id) !== String(conversationId))

  emit('update:selectedConversations', [...new Set(nextSelected)])
}
</script>

<style scoped>
.history-sidebar {
  display: flex;
  flex-direction: column;
  margin: 10px 10px 10px 0;
  padding: 12px;
  overflow: hidden;
  border: 1px solid var(--border-light);
  border-radius: var(--radius-md);
  background:
    linear-gradient(180deg, rgba(255, 255, 255, 0.96), rgba(245, 250, 251, 0.98));
}

.sidebar-header {
  padding: 20px;
  border: 1px solid rgba(8, 145, 178, 0.08);
  border-radius: 16px;
  background:
    radial-gradient(circle at top left, rgba(8, 145, 178, 0.08), transparent 35%),
    var(--ocean-pale);
}

.sidebar-eyebrow {
  margin: 0 0 6px;
  color: var(--teal-primary);
  font-size: 12px;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
}

.sidebar-header h3 {
  margin: 0;
  color: var(--text-primary);
  font-size: 16px;
  font-weight: 700;
}

.header-actions {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-top: 10px;
}

.history-list {
  flex: 1;
  overflow-y: auto;
  padding: 14px 6px 8px;
}

.conversation-item {
  margin-bottom: 12px;
  padding: 16px;
  border: 1px solid rgba(8, 145, 178, 0.1);
  border-radius: 16px;
  cursor: pointer;
  transition: all var(--transition);
  background-color: rgba(238, 247, 249, 0.76);
}

.conversation-item:hover {
  transform: translateY(-1px);
  border-color: rgba(8, 145, 178, 0.28);
  background-color: var(--teal-subtle);
}

.conversation-item.active {
  border-color: var(--teal-primary);
  background-color: rgba(221, 245, 250, 0.96);
  box-shadow: 0 8px 22px rgba(8, 145, 178, 0.12);
}

.conversation-content-wrapper {
  display: flex;
  align-items: center;
  width: 100%;
}

.hidden-label {
  display: none;
}

.conversation-details {
  width: 100%;
}

.conversation-details-manage {
  width: calc(100% - 30px);
  margin-left: 10px;
}

.conversation-title {
  margin-bottom: 8px;
  color: var(--text-primary);
  font-size: 14px;
  font-weight: 600;
}

.conversation-info {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 8px;
}

.conversation-time {
  color: #909399;
  font-size: 12px;
}

.conversation-preview {
  overflow: hidden;
  color: #606266;
  font-size: 12px;
  line-height: 1.5;
  text-overflow: ellipsis;
  display: -webkit-box;
  line-clamp: 2;
  -webkit-line-clamp: 2;
  -webkit-box-orient: vertical;
}

.sidebar-footer {
  padding: 10px 0 0;
  border-top: 1px solid var(--border-light);
}

.batch-delete-button {
  width: 100%;
}

.history-list::-webkit-scrollbar {
  width: 6px;
}

.history-list::-webkit-scrollbar-track {
  background: #f1f1f1;
  border-radius: 3px;
}

.history-list::-webkit-scrollbar-thumb {
  background: #c1c1c1;
  border-radius: 3px;
}

.history-list::-webkit-scrollbar-thumb:hover {
  background: #a8a8a8;
}
</style>
