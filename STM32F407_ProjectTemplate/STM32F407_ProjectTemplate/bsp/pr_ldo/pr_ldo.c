#include "pr_ldo.h"

// 全局变量定义
uint8_t pr_ldo_tx_buf[PR_LDO_TX_BUF_LEN] = {0};
uint8_t pr_ldo_rx_buf[PR_LDO_RX_BUF_LEN] = {0};
uint8_t pr_ldo_rx_cnt = 0;
uint8_t pr_ldo_rx_finish_flag = 0;
pr_ldo_data_t pr_ldo_data = {0};

// 浮点数转换联合体（大端模式适配）
typedef union
{
    uint8_t byte[4];
    float f_val;
    uint32_t u32_val;
} float_union_t;

/**
 * @brief  Modbus-RTU CRC16校验计算
 * @param  buf: 待计算的数据缓冲区
 * @param  len: 数据长度
 * @retval 计算得到的CRC16值
 */
static uint16_t pr_ldo_crc16(uint8_t *buf, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i = 0;
    uint8_t j = 0;

    for(i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for(j = 0; j < 8; j++)
        {
            if(crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief  传感器串口发送帧数据
 * @param  buf: 发送数据缓冲区
 * @param  len: 发送数据长度
 * @retval 无
 */
static void pr_ldo_send_frame(uint8_t *buf, uint8_t len)
{
    uint8_t i = 0;

    for(i = 0; i < len; i++)
    {
        while(USART_GetFlagStatus(PR_LDO_USART, USART_FLAG_TC) == RESET);
        USART_SendData(PR_LDO_USART, buf[i]);
    }
	
	while(USART_GetFlagStatus(PR_LDO_USART, USART_FLAG_TC) == RESET);
    delay_ms(2);
}

/**
 * @brief  清空接收缓冲区和状态
 * @param  无
 * @retval 无
 */
static void pr_ldo_clear_rx_buf(void)
{
    uint8_t i = 0;

    for(i = 0; i < PR_LDO_RX_BUF_LEN; i++)
    {
        pr_ldo_rx_buf[i] = 0;
    }
    pr_ldo_rx_cnt = 0;
    pr_ldo_rx_finish_flag = 0;
}

/**
 * @brief  写单个保持寄存器（通用底层函数）
 * @param  reg_addr: 寄存器地址
 * @param  data: 要写入的数据
 * @retval 0:成功 其他:失败
 */
static uint8_t pr_ldo_write_single_reg(uint16_t reg_addr, uint16_t data)
{
    uint8_t tx_len = 0;
    uint16_t crc_val = 0;
    uint32_t timeout = 0;

    // 清空接收缓冲区
    pr_ldo_clear_rx_buf();

    // 构造写单寄存器Modbus帧
    pr_ldo_tx_buf[tx_len++] = PR_LDO_DEFAULT_ADDR;
    pr_ldo_tx_buf[tx_len++] = PR_LDO_FUNC_WRITE_SINGLE;
    pr_ldo_tx_buf[tx_len++] = (reg_addr >> 8) & 0xFF;
    pr_ldo_tx_buf[tx_len++] = reg_addr & 0xFF;
    pr_ldo_tx_buf[tx_len++] = (data >> 8) & 0xFF;
    pr_ldo_tx_buf[tx_len++] = data & 0xFF;

    // 计算CRC并填充帧尾
    crc_val = pr_ldo_crc16(pr_ldo_tx_buf, tx_len);
    pr_ldo_tx_buf[tx_len++] = crc_val & 0xFF;
    pr_ldo_tx_buf[tx_len++] = (crc_val >> 8) & 0xFF;

    // 发送Modbus帧
    pr_ldo_send_frame(pr_ldo_tx_buf, tx_len);

    // 等待接收完成或超时
    timeout = PR_LDO_COMM_TIMEOUT;
    while(pr_ldo_rx_finish_flag == 0 && timeout > 0)
    {
        delay_ms(1);
        timeout--;
    }

    // 超时判断
    if(timeout == 0)
    {
        return 1;
    }

    // 校验应答帧长度
    if(pr_ldo_rx_cnt != 8)
    {
        return 2;
    }

    // 校验地址码
    if(pr_ldo_rx_buf[0] != PR_LDO_DEFAULT_ADDR)
    {
        return 3;
    }

    // 校验功能码
    if(pr_ldo_rx_buf[1] != PR_LDO_FUNC_WRITE_SINGLE)
    {
        return 4;
    }

    // 校验CRC
    crc_val = pr_ldo_crc16(pr_ldo_rx_buf, 6);
    if((pr_ldo_rx_buf[6] != (crc_val & 0xFF)) || (pr_ldo_rx_buf[7] != ((crc_val >> 8) & 0xFF)))
    {
        return 5;
    }

    // 写入成功
    return 0;
}

/**
 * @brief  传感器GPIO、串口和中断初始化
 * @param  无
 * @retval 无
 */
void pr_ldo_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    // 使能外设时钟
    RCC_AHB1PeriphClockCmd(PR_LDO_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(PR_LDO_USART_CLK, ENABLE);

    // 配置引脚复用映射
    GPIO_PinAFConfig(PR_LDO_GPIO_PORT, GPIO_PinSource5, PR_LDO_GPIO_AF);
    GPIO_PinAFConfig(PR_LDO_GPIO_PORT, GPIO_PinSource6, PR_LDO_GPIO_AF);

    // TX引脚初始化
    GPIO_InitStructure.GPIO_Pin = PR_LDO_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(PR_LDO_GPIO_PORT, &GPIO_InitStructure);

    // RX引脚初始化
    GPIO_InitStructure.GPIO_Pin = PR_LDO_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(PR_LDO_GPIO_PORT, &GPIO_InitStructure);

    // USART串口参数配置
    USART_InitStructure.USART_BaudRate = PR_LDO_DEFAULT_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(PR_LDO_USART, &USART_InitStructure);

    // 使能串口接收中断
    USART_ITConfig(PR_LDO_USART, USART_IT_RXNE, ENABLE);

    // 中断优先级配置
    NVIC_InitStructure.NVIC_IRQChannel = PR_LDO_USART_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能串口外设
    USART_Cmd(PR_LDO_USART, ENABLE);

    // 初始化传感器默认参数
    pr_ldo_data.salinity = 0;
    pr_ldo_data.atmos_press = 10133;
}

/**
 * @brief  读取传感器所有测量数据（溶解氧饱和度、浓度、温度）
 * @param  无
 * @retval 0:成功 其他:失败
 */
uint8_t pr_ldo_read_all_data(void)
{
    uint8_t tx_len = 0;
    uint16_t crc_val = 0;
    uint32_t timeout = 0;
//    uint8_t last_rx_cnt = 0;
//    uint8_t idle_time = 0;
    float_union_t float_union = {0};

    // 清空接收缓冲区
    pr_ldo_clear_rx_buf();

    // 构造读寄存器Modbus帧
    pr_ldo_tx_buf[tx_len++] = PR_LDO_DEFAULT_ADDR;
    pr_ldo_tx_buf[tx_len++] = PR_LDO_FUNC_READ_HOLD;
    pr_ldo_tx_buf[tx_len++] = 0x00;
    pr_ldo_tx_buf[tx_len++] = 0x00;
    pr_ldo_tx_buf[tx_len++] = 0x00;
    pr_ldo_tx_buf[tx_len++] = 0x06;

    // 计算CRC
    crc_val = pr_ldo_crc16(pr_ldo_tx_buf, tx_len);
    pr_ldo_tx_buf[tx_len++] = crc_val & 0xFF;
    pr_ldo_tx_buf[tx_len++] = (crc_val >> 8) & 0xFF;

    // 发送Modbus帧
    pr_ldo_send_frame(pr_ldo_tx_buf, tx_len);

    // 等待接收：只使用固定长度判断，最简单可靠
    timeout = PR_LDO_COMM_TIMEOUT;
    while(timeout > 0)
    {
        delay_ms(1);
        timeout--;
        
        // 收到17字节直接退出
        if(pr_ldo_rx_cnt == 17)
        {
            break;
        }
    }

    // 调试打印
//    printf("=== 调试信息 ===\r\n");
//    printf("接收字节数: %d\r\n", pr_ldo_rx_cnt);
//    printf("接收原始数据: ");
//    for(int i = 0; i < pr_ldo_rx_cnt; i++)
//    {
//        printf("%02X ", pr_ldo_rx_buf[i]);
//    }
//    printf("\r\n================\r\n");

    // 校验长度
    if(pr_ldo_rx_cnt != 17)
    {
        return 2;
    }

    // 校验CRC
    crc_val = pr_ldo_crc16(pr_ldo_rx_buf, 15);
    if((pr_ldo_rx_buf[15] != (crc_val & 0xFF)) || (pr_ldo_rx_buf[16] != ((crc_val >> 8) & 0xFF)))
    {
        return 6;
    }

    // 传感器发大端：[Byte0, Byte1, Byte2, Byte3] (MSB->LSB)
    // STM32是小端：需要存成 [Byte3, Byte2, Byte1, Byte0] (LSB->MSB)
    
    // 1. 解析溶解氧饱和度
    float_union.byte[0] = pr_ldo_rx_buf[6];
    float_union.byte[1] = pr_ldo_rx_buf[5];
    float_union.byte[2] = pr_ldo_rx_buf[4];
    float_union.byte[3] = pr_ldo_rx_buf[3];
    pr_ldo_data.do_saturation = float_union.f_val;

    // 2. 解析溶解氧浓度
    float_union.byte[0] = pr_ldo_rx_buf[10];
    float_union.byte[1] = pr_ldo_rx_buf[9];
    float_union.byte[2] = pr_ldo_rx_buf[8];
    float_union.byte[3] = pr_ldo_rx_buf[7];
    pr_ldo_data.do_concentration = float_union.f_val;

    // 3. 解析温度
    float_union.byte[0] = pr_ldo_rx_buf[14];
    float_union.byte[1] = pr_ldo_rx_buf[13];
    float_union.byte[2] = pr_ldo_rx_buf[12];
    float_union.byte[3] = pr_ldo_rx_buf[11];
    pr_ldo_data.temperature = float_union.f_val;
    // -----------------------------------------------------

    return 0;
}

/**
 * @brief  传感器零点校准
 * @param  无
 * @retval 0:成功 其他:失败
 */
uint8_t pr_ldo_calib_zero(void)
{
    return pr_ldo_write_single_reg(PR_LDO_REG_CALIB, PR_LDO_CALIB_ZERO);
}

/**
 * @brief  传感器满量程校准
 * @param  无
 * @retval 0:成功 其他:失败
 */
uint8_t pr_ldo_calib_full(void)
{
    return pr_ldo_write_single_reg(PR_LDO_REG_CALIB, PR_LDO_CALIB_FULL);
}

/**
 * @brief  设置传感器盐度值
 * @param  salinity: 盐度值，单位‰
 * @retval 0:成功 其他:失败
 */
uint8_t pr_ldo_set_salinity(uint16_t salinity)
{
    uint8_t ret = 0;

    ret = pr_ldo_write_single_reg(PR_LDO_REG_SALINITY, salinity);
    if(ret == 0)
    {
        pr_ldo_data.salinity = salinity;
    }
    return ret;
}

/**
 * @brief  设置传感器大气压力值
 * @param  press: 大气压力值，单位kPa*100（如101.35kPa写入10135）
 * @retval 0:成功 其他:失败
 */
uint8_t pr_ldo_set_atmos_press(uint16_t press)
{
    uint8_t ret = 0;

    ret = pr_ldo_write_single_reg(PR_LDO_REG_ATMOS_PRESS, press);
    if(ret == 0)
    {
        pr_ldo_data.atmos_press = press;
    }
    return ret;
}

/**
 * @brief  串口2中断服务函数
 * @param  无
 * @retval 无
 */
void PR_LDO_USART_IRQ_HANDLER(void)
{
    uint8_t rx_data = 0;

    // 1. 处理接收中断（RXNE）
    if(USART_GetITStatus(PR_LDO_USART, USART_IT_RXNE) != RESET)
    {
        rx_data = USART_ReceiveData(PR_LDO_USART);

        // 仅在未完成接收时存入缓冲区
        if(pr_ldo_rx_finish_flag == 0)
        {
            if(pr_ldo_rx_cnt < PR_LDO_RX_BUF_LEN)
            {
                pr_ldo_rx_buf[pr_ldo_rx_cnt++] = rx_data;
            }
        }

        // 注意：读取DR寄存器会自动清除RXNE标志，这里不需要手动清除
    }

    // 2. 查并清除所有错误标志位，防止中断锁死
    if(USART_GetFlagStatus(PR_LDO_USART, USART_FLAG_ORE) != RESET)
    {
        // 溢出错误：先读SR，再读DR，即可清除
        (void)PR_LDO_USART->SR;
        (void)PR_LDO_USART->DR;
    }
    if(USART_GetFlagStatus(PR_LDO_USART, USART_FLAG_NE) != RESET)
    {
        // 噪声错误：清除标志
        USART_ClearFlag(PR_LDO_USART, USART_FLAG_NE);
    }
    if(USART_GetFlagStatus(PR_LDO_USART, USART_FLAG_FE) != RESET)
    {
        // 帧错误：清除标志
        USART_ClearFlag(PR_LDO_USART, USART_FLAG_FE);
    }
    if(USART_GetFlagStatus(PR_LDO_USART, USART_FLAG_PE) != RESET)
    {
        // 校验错误：清除标志
        USART_ClearFlag(PR_LDO_USART, USART_FLAG_PE);
    }
}
