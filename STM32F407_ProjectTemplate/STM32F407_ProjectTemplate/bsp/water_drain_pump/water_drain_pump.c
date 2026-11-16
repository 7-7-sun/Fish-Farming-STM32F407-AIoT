
#include "water_drain_pump.h"

/**************************************************************************************
 * 函数名称 : water_drain_pump_init
 * 功能描述 : 排水泵GPIO初始化
 * 参    数 : 无
 * 返 回 值 : 无
 **************************************************************************************/
void water_drain_pump_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能GPIOD时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	      // 排水泵控制引脚为PD3
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     // 输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    // 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  // 无上下拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOD, GPIO_Pin_3);   // 初始状态置低电平，关闭排水泵
}

/**************************************************************************************
 * 函数名称 : water_drain_pump_on
 * 功能描述 : 开启排水泵
 * 参    数 : 无
 * 返 回 值 : 无
 **************************************************************************************/
void water_drain_pump_on(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_3);                      // 置高电平，开启排水泵
}

/**************************************************************************************
 * 函数名称 : water_drain_pump_off
 * 功能描述 : 关闭排水泵
 * 参    数 : 无
 * 返 回 值 : 无
 **************************************************************************************/
void water_drain_pump_off(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_3);                      // 置低电平，关闭排水泵
}

/**************************************************************************************
 * 函数名称 : water_drain_pump_get_status
 * 功能描述 : 获取排水泵当前运行状态
 * 参    数 : 无
 * 返 回 值 : 1-排水泵开启，0-排水泵关闭
 **************************************************************************************/
uint8_t water_drain_pump_get_status(void)
{
	// 读取GPIO引脚输出状态，1为开启，0为关闭
	if(GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_3) == Bit_SET)
	{
		return 1; // 排水泵处于开启状态
	}
	else
	{
		return 0; // 排水泵处于关闭状态
	}
}

/*********************************END FILE********************************************/