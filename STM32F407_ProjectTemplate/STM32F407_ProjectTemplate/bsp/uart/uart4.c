#include "uart4.h"
#include "stdio.h"
#include "string.h"
#include "bsp_uart.h"

// 环形缓冲区全局变量
static uint8_t uart4_rx_buffer[UART4_RX_BUFFER_SIZE];
static volatile uint16_t uart4_rx_write_index = 0;
static volatile uint16_t uart4_rx_read_index = 0;

void uart4_init(uint32_t __Baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(UART4_GPIO_CLK, ENABLE);

	GPIO_PinAFConfig(UART4_GPIO_PORT, GPIO_PinSource10, UART4_GPIO_AF);
	GPIO_PinAFConfig(UART4_GPIO_PORT, GPIO_PinSource11, UART4_GPIO_AF);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin           = UART4_TX_PIN;
	GPIO_InitStructure.GPIO_Mode          = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed         = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType         = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd          = GPIO_PuPd_UP;
	GPIO_Init(UART4_GPIO_PORT, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin           = UART4_RX_PIN;
	GPIO_InitStructure.GPIO_Mode          = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed         = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType         = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd          = GPIO_PuPd_UP;
	GPIO_Init(UART4_GPIO_PORT, &GPIO_InitStructure);

	USART_InitTypeDef USART_InitStructure;

	RCC_APB1PeriphClockCmd(UART4_USART_CLK, ENABLE);

	USART_DeInit(UART4_USART);

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate              = __Baud;
	USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                = USART_Parity_No;
	USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
	USART_Init(UART4_USART, &USART_InitStructure);

	USART_ClearFlag(UART4_USART, USART_FLAG_RXNE);

	USART_ITConfig(UART4_USART, USART_IT_RXNE, ENABLE);

	USART_Cmd(UART4_USART, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel                    = UART4_USART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/******** 串口4 中断服务函数 ***********/
void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(UART4_USART, USART_IT_RXNE) == SET)
	{
		// 接收数据并存入环形缓冲区
		uint8_t data = USART_ReceiveData(UART4_USART);

		// 计算下一个写位置
		uint16_t next_write_index = (uart4_rx_write_index + 1) % UART4_RX_BUFFER_SIZE;

		// 检查缓冲区是否已满（写指针+1等于读指针）
		if(next_write_index != uart4_rx_read_index)
		{
			// 缓冲区未满，存入数据
			uart4_rx_buffer[uart4_rx_write_index] = data;
			uart4_rx_write_index = next_write_index;
		}
		// 如果缓冲区已满，数据将被丢弃

		USART_ClearITPendingBit(UART4_USART, USART_IT_RXNE);
	}
}

/******** 串口4 测试发送函数 ***********/
void uart4_send_string(const char *str)
{
	while(*str)
	{
		USART_SendData(UART4_USART, (uint8_t)(*str));
		while(USART_GetFlagStatus(UART4_USART, USART_FLAG_TXE) == RESET);
		str++;
	}
}

/******** 串口4 接收数据处理函数 ***********/

// 从环形缓冲区读取一个字节的数据
// 返回值：1表示成功读取，0表示缓冲区为空
uint8_t uart4_get_rx_data(uint8_t *data)
{
	// 检查缓冲区是否为空（读指针等于写指针）
	if(uart4_rx_read_index == uart4_rx_write_index)
	{
		return 0; // 缓冲区为空
	}

	*data = uart4_rx_buffer[uart4_rx_read_index];
	uart4_rx_read_index = (uart4_rx_read_index + 1) % UART4_RX_BUFFER_SIZE;

	return 1; // 成功读取
}

// 将UART4接收到的数据通过UART1打印出来
void uart4_echo_to_uart1(void)
{
	uint8_t data;
	while(uart4_get_rx_data(&data))
	{
		// 使用uart1_printf发送到UART1
		// 这里我们直接使用USART_SendData发送单个字符
		USART_SendData(USART1, data);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
}
