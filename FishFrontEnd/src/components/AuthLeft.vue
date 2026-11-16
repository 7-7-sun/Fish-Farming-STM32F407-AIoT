<template>
  <div class="auth-left">
    <img
      src="/loginBg.png"
      alt="Auth Background"
      class="bg-image" />
    <div class="overlay"></div>

    <div class="left-content">
      <div class="brand">
        <div class="brand-logo">
          <svg
            width="24"
            height="24"
            viewBox="0 0 24 24"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            stroke-linecap="round"
            stroke-linejoin="round">
            <path
              d="M2 12c.6.5 1.2 1 2.5 1 2.5 0 2.5-2 5-2 2.6 0 2.4 2 5 2 2.5 0 2.5-2 5-2 1.3 0 1.9.5 2.5 1" />
            <path
              d="M2 17c.6.5 1.2 1 2.5 1 2.5 0 2.5-2 5-2 2.6 0 2.4 2 5 2 2.5 0 2.5-2 5-2 1.3 0 1.9.5 2.5 1" />
            <path
              d="M2 7c.6.5 1.2 1 2.5 1 2.5 0 2.5-2 5-2 2.6 0 2.4 2 5 2 2.5 0 2.5-2 5-2 1.3 0 1.9.5 2.5 1" />
          </svg>
        </div>
        <span class="brand-name">鱼类养殖变量投喂生态系统</span>
      </div>

      <div class="hero-text">
        <h1>观察. <span class="highlight">分析.</span> 优化.</h1>
        <p>利用人工智能驱动的行为分析，监测水产养殖系统。</p>
      </div>

      <div class="stats-row">
        <div class="stat-card">
          <div class="stat-value">99.8%</div>
          <div class="stat-label">识别准确度</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">实时</div>
          <div class="stat-label">响应延迟</div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.auth-left {
  flex: 0 0 45%;
  position: relative;
  overflow: hidden;
  background-color: var(--ocean-deep);
}

.bg-image {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  object-fit: cover;
  opacity: 0.6;
}

.overlay {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: linear-gradient(
    180deg,
    rgba(10, 22, 40, 0.3) 0%,
    rgba(10, 22, 40, 0.85) 70%,
    rgba(10, 22, 40, 0.95) 100%
  );
}

.left-content {
  position: relative;
  z-index: 2;
  height: 100%;
  padding: 60px;
  display: flex;
  flex-direction: column;
  justify-content: flex-start;
  color: white;
}

.brand {
  display: flex;
  align-items: center;
  gap: 12px;
  margin-bottom: 80px;
}

.brand-logo {
  width: 44px;
  height: 44px;
  background: linear-gradient(135deg, var(--teal-primary), var(--teal-light));
  border-radius: var(--radius-sm);
  display: flex;
  align-items: center;
  justify-content: center;
}

.brand-name {
  font-size: 1.15rem;
  font-weight: 700;
  letter-spacing: 0.5px;
}

.hero-text h1 {
  font-size: 3.25rem;
  font-weight: 800;
  line-height: 1.2;
  margin-bottom: 24px;
}

.hero-text .highlight {
  color: var(--teal-light);
}

.hero-text p {
  font-size: 1.05rem;
  color: rgba(148, 200, 224, 0.7);
  max-width: 450px;
  line-height: 1.7;
}

.stats-row {
  margin-top: auto;
  display: flex;
  gap: 20px;
}

.stat-card {
  flex: 1;
  background: rgba(8, 145, 178, 0.08);
  border: 1px solid rgba(8, 145, 178, 0.15);
  padding: 24px;
  border-radius: var(--radius-md);
  backdrop-filter: blur(12px);
}

.stat-value {
  font-size: 1.5rem;
  font-weight: 700;
  margin-bottom: 4px;
  color: var(--teal-light);
}

.stat-label {
  font-size: 0.75rem;
  color: rgba(148, 200, 224, 0.5);
  text-transform: uppercase;
  letter-spacing: 1px;
}

@media (max-width: 1024px) {
  .auth-left {
    display: none;
  }
}
</style>
