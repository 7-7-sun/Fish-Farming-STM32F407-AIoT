#include "ds18b20.h"
#include "board.h"

unsigned char DS18B20_Init(void)
{
    unsigned char ret = 255;

    RCC_AHB1PeriphClockCmd(RCC_DQ, ENABLE); // 使能GPIO时钟

    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_DQ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(PORT_DQ, &GPIO_InitStructure);

    ret = DS18B20_Check();  // 检测器件是否存在

    return ret;
}

unsigned char DS18B20_Check(void)
{
    unsigned char retry = 0;

    DS18B20_Reset();

    DQ_IN();

    // 等待温度传感器拉低电平
    while(DQ_GET() && retry < 200)
    {
        delay_us(1);
        retry++;
    }

    if(retry >= 200)
    {
        return 1;
    }
    else
    {
        retry = 0;
    }

    // 等待释放总线
    while(!DQ_GET() && retry < 240)
    {
        delay_us(1);
        retry++;
    }
    
    if(retry >= 240)
    {
        return 1;
    }

    return 0;
}

void DS18B20_Reset(void)
{
    DQ_OUT(); // 设置为输出模式
    DQ(0); // 拉低DQ
    delay_us(750); // 拉低750us
    DQ(1); // 拉高DQ
    delay_us(15);  // 15us
}

unsigned char DS18B20_Read_Byte(void)
{
    unsigned char dat = 0;

     for(int i = 0;i < 8;i++)
     {
        DQ_OUT();
        DQ(0);
        delay_us(2);
        DQ(1);
        DQ_IN();
        delay_us(12);
        dat >>= 1;

        if(DQ_GET())
        {
            dat = dat | 0x80;        // 低位先行
        }

        delay_us(50);
     }

     return dat;
}

void DS18B20_Write_Byte(unsigned char dat)
{
    DQ_OUT();

    for(int i = 0;i < 8;i++)
    {
        // 低位先行
        if(dat & 0x01)          // 写1
        {
            DQ(0);
            delay_us(2);
            DQ(1);
            delay_us(60);
        }
        else                    // 写0
        {
            DQ(0);
            delay_us(60);
            DQ(1);
            delay_us(2);
        }
        dat = dat >> 1;
    }
}

void DS18B20_Start(void)
{
    DS18B20_Check();            // 查询是否有设备应答
    DS18B20_Write_Byte(0xCC);   // 对总线上所有设备进行寻址
    DS18B20_Write_Byte(0x44);   // 启动温度转换
}

float DS18B20_Get_Temperature(void)
{
    float result = 0.0;         // 保存最终温度值
    int temp = 0;
    unsigned char data_h = 0;   // 高位数据
    unsigned char data_l = 0;   // 低位数据

    DS18B20_Start();
    DS18B20_Check();
    DS18B20_Write_Byte(0xCC);
    DS18B20_Write_Byte(0xBE);

    data_l = DS18B20_Read_Byte();       // 先发低8位数据
    data_h = DS18B20_Read_Byte();

    // 组合成16位有符号整数
    temp = (int16_t)((data_h << 8) | data_l);
    
    // 计算温度
    result = (float)(temp * 0.0625);

    return result;
}
