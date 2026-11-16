#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdio.h>
#include "stdint.h"

#include "stm32f4xx.h"
#include "scheduler.h"

#include "led.h"
#include "oled.h"
#include "bsp_uart.h"
#include "adc.h"
#include "ds18b20.h"
#include "tim6.h"
#include "pr_ldo.h"
#include "water_pump.h"
#include "heating_rod.h"

void board_init(void);
void delay_us(uint32_t _us);
void delay_ms(uint32_t _ms);
void delay_1ms(uint32_t ms);
void delay_1us(uint32_t us);

extern uint32_t tim6_count;

#endif
