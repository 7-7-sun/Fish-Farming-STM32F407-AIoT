#include "tim6.h"

uint32_t tim6_count;

/**
 * @brief  TIM6 初始化函数：配置 1ms 中断
 * @param  无
 * @retval 无
 */
void tim6_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. 使能 TIM6 时钟（APB1 外设时钟）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    // 2. 配置 TIM6 时基参数
    // 预分频器：84MHz / 84 = 1MHz（计数器每 1us 加 1）
    TIM_TimeBaseInitStruct.TIM_Prescaler = 84 - 1; 
    // 时钟分频：不分频（基本定时器无此功能，仅占位）
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    // 计数模式：向上计数（基本定时器仅支持向上计数）
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    // 自动重装值：1000 - 1 → 计数到 1000 溢出（1us × 1000 = 1ms）
    TIM_TimeBaseInitStruct.TIM_Period = 1000 - 1;
    // 重复计数器：基本定时器无此功能，设为 0
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    
    // 应用时基配置
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);

    // 3. 使能 TIM6 更新中断（溢出中断）
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

    // 4. 配置 NVIC 中断优先级（核心：确保中断能被响应）
    // 抢占优先级：1（可根据项目调整）
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    // 子优先级：0（可根据项目调整）
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    // 使能 TIM6 中断通道
    NVIC_InitStruct.NVIC_IRQChannel = TIM6_DAC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 5. 清除更新中断标志位（防止刚初始化就触发中断）
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

    // 6. 使能 TIM6 计数器
    TIM_Cmd(TIM6, ENABLE);
}

/**
 * @brief  TIM6 中断服务函数
 * @param  无
 * @retval 无
 */
void TIM6_DAC_IRQHandler(void)
{
    // 检查更新中断标志位（防止误触发）
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
    {
        tim6_count++;
		
        // 清除中断标志位
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
