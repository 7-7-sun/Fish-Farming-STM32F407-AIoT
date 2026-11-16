#ifndef __PR_LDO_H
#define __PR_LDO_H

#include "board.h"

// 传感器默认配置
#define PR_LDO_DEFAULT_ADDR        0x01
#define PR_LDO_DEFAULT_BAUDRATE    4800

// Modbus功能码定义
#define PR_LDO_FUNC_READ_HOLD      0x03
#define PR_LDO_FUNC_WRITE_SINGLE   0x06

// 寄存器地址定义
#define PR_LDO_REG_DO_SAT_H        0x0000  // 溶解氧饱和度高寄存器
#define PR_LDO_REG_DO_CONC_H       0x0002  // 溶解氧浓度高寄存器
#define PR_LDO_REG_TEMP_H          0x0004  // 温度高寄存器
#define PR_LDO_REG_CALIB           0x1010  // 校准寄存器
#define PR_LDO_REG_SALINITY        0x1020  // 盐度寄存器
#define PR_LDO_REG_ATMOS_PRESS     0x1022  // 大气压力寄存器

// 校准命令定义
#define PR_LDO_CALIB_ZERO          0x0001  // 零点校准命令
#define PR_LDO_CALIB_FULL          0x0002  // 满量程校准命令

// 串口硬件相关定义
#define PR_LDO_USART               USART2
#define PR_LDO_USART_CLK           RCC_APB1Periph_USART2
#define PR_LDO_USART_IRQ           USART2_IRQn
#define PR_LDO_USART_IRQ_HANDLER   USART2_IRQHandler

#define PR_LDO_GPIO_PORT           GPIOD
#define PR_LDO_GPIO_CLK            RCC_AHB1Periph_GPIOD
#define PR_LDO_TX_PIN              GPIO_Pin_5
#define PR_LDO_RX_PIN              GPIO_Pin_6
#define PR_LDO_GPIO_AF             GPIO_AF_USART2

// 缓冲区长度定义
#define PR_LDO_TX_BUF_LEN          64
#define PR_LDO_RX_BUF_LEN          64

// 通信超时时间定义(ms)
#define PR_LDO_COMM_TIMEOUT        500
#define PR_LDO_FRAME_IDLE_TIME     10

// 传感器数据结构体
typedef struct
{
    float do_saturation;       // 溶解氧饱和度，单位%
    float do_concentration;    // 溶解氧浓度，单位mg/L
    float temperature;         // 温度，单位℃
    uint16_t salinity;         // 盐度，单位‰
    uint16_t atmos_press;      // 大气压力，单位kPa*100
} pr_ldo_data_t;

// 全局变量声明
extern uint8_t pr_ldo_tx_buf[PR_LDO_TX_BUF_LEN];
extern uint8_t pr_ldo_rx_buf[PR_LDO_RX_BUF_LEN];
extern uint8_t pr_ldo_rx_cnt;
extern uint8_t pr_ldo_rx_finish_flag;
extern pr_ldo_data_t pr_ldo_data;

// 函数声明
void pr_ldo_init(void);
uint8_t pr_ldo_read_all_data(void);
uint8_t pr_ldo_calib_zero(void);
uint8_t pr_ldo_calib_full(void);
uint8_t pr_ldo_set_salinity(uint16_t salinity);
uint8_t pr_ldo_set_atmos_press(uint16_t press);

#endif
