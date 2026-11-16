#include "board.h"
#include "uart4.h"
#include "water_pump.h"
#include "water_add_pump.h"
#include "water_drain_pump.h"

int main(void)
{
	board_init();           // 开发板基础硬件初始化

	scheduler_init();       // 任务调度器初始化

	uart1_init(115200U);    // 串口1初始化 (波特率115200)
	uart4_init(115200U);    // 串口4初始化 (波特率115200)

	LED_Init();             // LED指示灯初始化

	OLED_Init();            // OLED屏幕初始化
	OLED_Clear();           // 清屏

	tim6_init();            // 定时器6初始化 (通常用于提供系统滴答或任务调度心跳)

	DS18B20_Init();		    // DS18B20温度传感器初始化
	ADC1_Init();		    // ADC1初始化 (用于采集模拟量传感器数据，如浊度或水位)
	ADC2_Init();		    // pH传感器初始化 (通过ADC2采集)

	pr_ldo_init();		    // 传感器LDO电源初始化 (用于控制外设传感器的供电)

	water_pump_init();		// 氧气泵初始化
	water_add_pump_init();	// 加水泵初始化
	water_drain_pump_init();// 排水泵初始化
	heating_rod_init();		// 加热棒初始化

	// 以下为测试代码，正常运行时通常交由业务逻辑或状态机去控制
	//water_pump_on();		// 开启水泵
	//water_add_pump_on();	// 开启加水泵
	//water_drain_pump_on();// 开启排水泵
	//heating_rod_on();		// 开启加热棒

	// 测试UART4通信发送
	uart4_send_string("UART4 Test Start\r\n");

	for(;;)
	{
		scheduler_run();    // 执行任务调度器轮询
	}
}