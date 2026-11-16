import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import AutoImport from 'unplugin-auto-import/vite'
import Components from 'unplugin-vue-components/vite'
import { ElementPlusResolver } from 'unplugin-vue-components/resolvers'


export default defineConfig({
  base: '/fish/',
  plugins: [
    vue(),
    AutoImport({
      imports: ['vue', 'vue-router'],
      resolvers: [ElementPlusResolver()],
    }),
    // 自动导入组件
    Components({
      resolvers: [
        // 处理 Element Plus
        ElementPlusResolver({
          importStyle: 'sass',  // 使用 sass 而不是 css
        }),
      ],
    })],
    server: {
      proxy: {
        '/api': {
          target: 'http://139.9.0.201:9000',  // 后端地址
          changeOrigin: true,               // 改变源
        },
      }
    }
})
