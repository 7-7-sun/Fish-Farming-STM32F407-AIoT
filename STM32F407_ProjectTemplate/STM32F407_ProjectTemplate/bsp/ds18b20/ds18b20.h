#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stm32f4xx.h"

#define RCC_DQ RCC_AHB1Periph_GPIOB
#define PORT_DQ GPIOB
#define GPIO_DQ GPIO_Pin_0

// 设置DQ为输出模式
#define DQ_OUT() {  \
                        GPIO_InitTypeDef  GPIO_InitStructure; \
                        GPIO_InitStructure.GPIO_Pin = GPIO_DQ; \
                        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; \
                        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; \
                        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; \
                        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; \
                        GPIO_Init(PORT_DQ, &GPIO_InitStructure); \
                }

// 设置DQ为输入模式
#define DQ_IN() {   \
                    GPIO_InitTypeDef  GPIO_InitStructure; \
                    GPIO_InitStructure.GPIO_Pin = GPIO_DQ; \
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; \
                    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; \
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; \
                    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; \
                    GPIO_Init(PORT_DQ, &GPIO_InitStructure); \
                }

// 获取DQ输入
#define DQ_GET() GPIO_ReadInputDataBit(PORT_DQ, GPIO_DQ)

// 输出DQ电平
#define DQ(x) GPIO_WriteBit(PORT_DQ, GPIO_DQ, (x?Bit_SET:Bit_RESET))

unsigned char DS18B20_Init(void);
unsigned char DS18B20_Check(void);
void DS18B20_Reset(void);
unsigned char DS18B20_Read_Byte(void);
void DS18B20_Write_Byte(unsigned char dat);
void DS18B20_Start(void);
float DS18B20_Get_Temperature(void);

#endif
