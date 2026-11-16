#ifndef __ADC_H__
#define __ADC_H__

typedef unsigned char uint8_t;

void ADC1_Init(void);
void ADC2_Init(void);
unsigned int Get_ADC1_Average(uint8_t ADC_CHANNEL_x, uint8_t times);
float Get_ADC2_Average(uint8_t ADC_CHANNEL_x, uint8_t times);

#endif
