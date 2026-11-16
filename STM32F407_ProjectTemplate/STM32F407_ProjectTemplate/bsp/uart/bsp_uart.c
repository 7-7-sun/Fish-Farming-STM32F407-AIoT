#include "bsp_uart.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "uart4.h"

void uart1_init(uint32_t __Baud)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // 使能串口1时钟

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    // TX与RX引脚可以合并初始化，减少代码冗余
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9 | GPIO_Pin_10; 
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_DeInit(USART1); // 复位串口寄存器
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate            = __Baud;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t data = USART_ReceiveData(USART1); // 读取动作会自动清除RXNE标志位
        
        // 接收回显
        USART_SendData(USART1, data);
        
        // 建议加上超时保护，防止死机
        uint16_t timeout = 0xFFFF;
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET && timeout--);
    }
}

// 标准 printf 重定向到 UART1
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    uint16_t timeout = 0xFFFF;
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET && timeout--);
    return ch;
}

// UART4 保留独立打印，但将 buffer 缩小到合理范围
void uart4_printf(const char *format, ...)
{
    char buffer[128]; // 从256改为128，节约栈空间
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    char *p = buffer;
    while(*p)
    {
        USART_SendData(UART4_USART, (uint8_t)*p);
        
        // 建议加上超时保护
        uint16_t timeout = 0xFFFF;
        while(USART_GetFlagStatus(UART4_USART, USART_FLAG_TXE) == RESET && timeout--);
        p++;
    }
}