/*
 * 本开源项目由LCKFB团队开发维护，提供完整软硬件资源
 * 官方网站：www.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 技术论坛：https://oshwhub.com/forum
 * 关注bilibili账号：【LCKFB】获取最新项目动态
 * 感谢打赏支持，中国工程师为您服务
 *

 Change Logs:
 * Date           Author       Notes
 * 2026-03-28     Assistant    add uart4 driver
 */

#ifndef __UART4_H__
#define __UART4_H__

#include "stm32f4xx.h"

// 硬件相关定义
#define UART4_USART               UART4
#define UART4_USART_CLK           RCC_APB1Periph_UART4
#define UART4_USART_IRQ           UART4_IRQn
#define UART4_USART_IRQ_HANDLER   UART4_IRQHandler

#define UART4_GPIO_PORT           GPIOC
#define UART4_GPIO_CLK            RCC_AHB1Periph_GPIOC
#define UART4_TX_PIN              GPIO_Pin_10
#define UART4_RX_PIN              GPIO_Pin_11
#define UART4_GPIO_AF             GPIO_AF_UART4

// 环形缓冲区相关定义
#define UART4_RX_BUFFER_SIZE 256

// 外部可调用函数声明
void uart4_init(uint32_t __Baud);
void UART4_IRQHandler(void);
void uart4_send_string(const char *str);

// 新增：UART4接收数据处理函数
uint8_t uart4_get_rx_data(uint8_t *data);
void uart4_echo_to_uart1(void);

#endif