<script setup></script>

<template>
  <div id="app">
    <router-view />
  </div>
</template>

<style>
/* 全局基础样式 */
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body,
html,
#app {
  width: 100%;
  height: 100%;
  /* overflow: hidden; */
}
</style>
