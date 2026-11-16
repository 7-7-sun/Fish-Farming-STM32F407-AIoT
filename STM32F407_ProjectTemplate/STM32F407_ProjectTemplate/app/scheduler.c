#include "scheduler.h"
#include "bsp_uart.h"
#include "uart4.h"
#include "water_pump.h"
#include "water_add_pump.h"
#include "water_drain_pump.h"
#include "heating_rod.h"

//定义全局变量，用于暂存传感器最新数据
float g_temp = 0.00;
float g_turbidity = 0.00;
float g_ph = 0.00;
int g_atmos_press = 0;
float g_do_saturation = 0.00;
float g_do_concentration = 0.00;
uint8_t g_ldo_status = 1; // 记录 LDO 传感器状态，0 为正常

uint8_t g_heating_rod_status = 0; // 加热棒状态：1为开启，0为关闭
uint8_t g_air_pump_status = 0;     // 充气泵状态：1为开启，0为关闭
uint8_t g_water_add_pump_status = 0;  // 加水泵状态：1为开启，0为关闭
uint8_t g_water_drain_pump_status = 0; // 抽水泵状态：1为开启，0为关闭

// ----------task----------
// ------------------------

//采集任务只更新全局变量
void temperature_get_proc(void)
{
	g_temp = DS18B20_Get_Temperature();
}

void turbidity_get_proc(void)
{
	unsigned int turbidity_value = Get_ADC1_Average(5, 5);
	g_turbidity = (float)(((turbidity_value / 4096.0f * 3.3f) * (5.0f / 3.0f)) * -865.68f + 3165.7571328f + 347.1f);
}

void ph_get_proc(void)
{
	float ph_value = Get_ADC2_Average(1, 5);
	g_ph = ((ph_value / 4096.0f * 3.3f) * (-5.7541f)) + 16.654f;
}

void pr_ldo_proc(void)
{
	g_ldo_status = pr_ldo_read_all_data();
	if(g_ldo_status == 0)
	{
		g_atmos_press = pr_ldo_data.atmos_press;
		g_do_saturation = pr_ldo_data.do_saturation;
		g_do_concentration = pr_ldo_data.do_concentration;
	}
}

// 获取加热棒和所有水泵状态任务
void device_status_proc(void)
{
	// 获取加热棒状态
	g_heating_rod_status = heating_rod_get_status();

	// 获取充气泵状态
	g_air_pump_status = water_pump_get_status();

	// 获取加水泵状态
	g_water_add_pump_status = water_add_pump_get_status();

	// 获取抽水泵状态
	g_water_drain_pump_status = water_drain_pump_get_status();
}

// JSON 数据打包发送任务
// 逐个发送数据的任务（状态机模式）
// 轮询发送任务：完全匹配 ESP32 的 KEY:VALUE\n 格式
void data_upload_proc(void)
{
    // 发送传感器数据
    uart4_printf("TEMP:%.2f\n", g_temp);
    uart4_printf("TURB:%.1f\n", g_turbidity);
    uart4_printf("PH:%.2f\n", g_ph);
    uart4_printf("DO_CONC:%.1f\n", g_do_concentration);

    // 发送设备状态数据
    uart4_printf("HEAT:%d\n", g_heating_rod_status);
    uart4_printf("ADD_PUMP:%d\n", g_water_add_pump_status);
    uart4_printf("DRAIN_PUMP:%d\n", g_water_drain_pump_status);
    uart4_printf("AIR_PUMP:%d\n", g_air_pump_status);
	
	    // 发送传感器数据
    printf("TEMP:%.2f\n", g_temp);
    printf("TURB:%.1f\n", g_turbidity);
    printf("PH:%.2f\n", g_ph);
    printf("DO_CONC:%.1f\n", g_do_concentration);

    // 发送设备状态数据
    printf("HEAT:%d\n", g_heating_rod_status);
    printf("ADD_PUMP:%d\n", g_water_add_pump_status);
    printf("DRAIN_PUMP:%d\n", g_water_drain_pump_status);
    printf("AIR_PUMP:%d\n", g_air_pump_status);
	
}

// UART4接收数据回显任务
void uart4_echo_proc(void)
{
	// 将UART4接收到的数据通过UART1打印出来
	uart4_echo_to_uart1();
}

//将接收到的云端指令通过串口1回显输出
void log_cloud_command_to_uart1(const char* cmd)
{
	printf("[ESP32 -> STM32]: %s\r\n", cmd);
}

// UART4指令处理任务
void uart4_command_processor(void)
{
	static char command_buffer[64];
	static uint8_t buffer_index = 0;
	uint8_t data;

	while(uart4_get_rx_data(&data))
	{
		// 检查是否为换行符（表示指令结束）
		if (data == '\n' || data == '\r') {
			if (buffer_index > 0) {
				// 添加字符串结束符，此时 command_buffer 是一条完整的指令
				command_buffer[buffer_index] = '\0';

				// ==========================================
				// 在此处调用新建的函数，将完整指令输出到串口1
				log_cloud_command_to_uart1(command_buffer);
				// ==========================================

				// 使用strstr()函数查找指令关键字，执行相应的动作
				if (strstr(command_buffer, "CMD_HEATER:1")) {
					heating_rod_on();
				}
				else if (strstr(command_buffer, "CMD_HEATER:0")) {
					heating_rod_off();
				}
				else if (strstr(command_buffer, "CMD_PUMP_IN:1")) {
					water_add_pump_on();
				}
				else if (strstr(command_buffer, "CMD_PUMP_IN:0")) {
					water_add_pump_off();
				}
				else if (strstr(command_buffer, "CMD_PUMP_OUT:1")) {
					water_drain_pump_on();
				}
				else if (strstr(command_buffer, "CMD_PUMP_OUT:0")) {
					water_drain_pump_off();
				}
				else if (strstr(command_buffer, "CMD_AIR_PUMP:1")) {
					water_pump_on();
				}
				else if (strstr(command_buffer, "CMD_AIR_PUMP:0")) {
					water_pump_off();
				}

				// 重置缓冲区，准备接收下一条指令
				buffer_index = 0;
			}
		}
		else if (buffer_index < sizeof(command_buffer) - 1) {
			// 将字符存入缓冲区
			command_buffer[buffer_index++] = data;
		}
		else {
			// 缓冲区溢出，重置缓冲区
			buffer_index = 0;
			// 这里的溢出警告也可以顺便改成发给串口1
			printf("[STM32] 警告：指令缓冲区溢出，已清空\n"); 
		}
	}
}

typedef struct
{
	uint32_t last_tick;		// 上一次运行时间
	uint32_t rate_tick;		// 任务周期
	void (*func)(void);		// 任务函数
} task_t;

task_t tasks[] =
{
    {0, 200, temperature_get_proc}, 
    {0, 200, turbidity_get_proc},
    {0, 200, ph_get_proc},
    {0, 200, pr_ldo_proc},
    {0, 500, device_status_proc},   
    {0, 1000, data_upload_proc},     // 每秒钟一口气按顺序吐出所有数据
    {0, 50, uart4_command_processor},  
};

uint8_t task_count;

void scheduler_init(void)
{
	task_count = sizeof(tasks) / sizeof(task_t);
}

void scheduler_run(void)
{
	for(int i = 0;i < task_count;i++)
	{
		uint32_t now_tick = tim6_count;
		if(now_tick - tasks[i].last_tick >= tasks[i].rate_tick)
		{
			tasks[i].func();
			tasks[i].last_tick = now_tick;
		}
	}
}
