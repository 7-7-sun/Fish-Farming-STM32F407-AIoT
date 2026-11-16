#include "board.h"

void ADC1_GPIO_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure; // 配置结构体

    RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE); // 使能时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;    // 模拟输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    GPIO_Init(GPIOA, &GPIO_InitStructure); // 初始化GPIO
}

void ADC2_GPIO_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure; // 配置结构体

    RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE); // 使能时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;    // 模拟输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    GPIO_Init(GPIOA, &GPIO_InitStructure); // 初始化GPIO
}

void ADC1_Init(void)
{
	ADC1_GPIO_Init();
	
    // ADC公共配置
    ADC_CommonInitTypeDef ADC_CommonInitStruct;

    ADC_DeInit();   // ADC复位

    ADC_CommonInitStruct.ADC_DMAAccessMode=ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStruct.ADC_Mode=ADC_Mode_Independent;
    ADC_CommonInitStruct.ADC_Prescaler=ADC_Prescaler_Div4;
    ADC_CommonInitStruct.ADC_TwoSamplingDelay=ADC_TwoSamplingDelay_5Cycles;

    ADC_CommonInit(&ADC_CommonInitStruct);

    // ADC1特定配置
    ADC_InitTypeDef ADC_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;
    ADC_InitStruct.ADC_ExternalTrigConvEdge=ADC_ExternalTrigConvEdge_None;
    ADC_InitStruct.ADC_NbrOfConversion=1;
    ADC_InitStruct.ADC_Resolution=ADC_Resolution_12b;
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;

    ADC_Init(ADC1, &ADC_InitStruct);

    ADC_Cmd(ADC1, ENABLE);
}

void ADC2_Init(void)
{
	ADC2_GPIO_Init();
	
    // ADC1特定配置
    ADC_InitTypeDef ADC_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;
    ADC_InitStruct.ADC_ExternalTrigConvEdge=ADC_ExternalTrigConvEdge_None;
    ADC_InitStruct.ADC_NbrOfConversion=1;
    ADC_InitStruct.ADC_Resolution=ADC_Resolution_12b;
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;

    ADC_Init(ADC2, &ADC_InitStruct);

    ADC_Cmd(ADC2, ENABLE);
}

unsigned int Get_ADC1_Value(uint8_t  ADC_CHANNEL_x)
{
    unsigned int adc_value = 0;

    // 设置采集通道
    ADC_RegularChannelConfig(ADC1, ADC_CHANNEL_x, 1, ADC_SampleTime_480Cycles);
    // 开始软件转换
    ADC_SoftwareStartConv(ADC1);
    // 等待转换结束
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    // 读取采样值
    adc_value = ADC_GetConversionValue(ADC1);

    // 返回采样值
    return adc_value;
}

unsigned int Get_ADC2_Value(uint8_t  ADC_CHANNEL_x)
{
    unsigned int adc_value = 0;

    // 设置采集通道
    ADC_RegularChannelConfig(ADC2, ADC_CHANNEL_x, 1, ADC_SampleTime_480Cycles);
    // 开始软件转换
    ADC_SoftwareStartConv(ADC2);
    // 等待转换结束
    while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC));

    // 读取采样值
    adc_value = ADC_GetConversionValue(ADC2);

    // 返回采样值
    return adc_value;
}

unsigned int Get_ADC1_Average(uint8_t ADC_CHANNEL_x, uint8_t times)
{
    unsigned int value = 0;

    for(int i = 0;i <times; i++)
    {
        value += Get_ADC1_Value(ADC_CHANNEL_x);

        delay_ms(5);
    }

    return value / times;
}

float Get_ADC2_Average(uint8_t ADC_CHANNEL_x, uint8_t times)
{
    unsigned int value = 0;

    for(int i = 0;i <times; i++)
    {
        value += Get_ADC2_Value(ADC_CHANNEL_x);

        delay_ms(5);
    }

    return (float)value / (float)times;
}
