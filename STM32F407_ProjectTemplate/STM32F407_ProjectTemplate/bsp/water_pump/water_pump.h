#ifndef __WATER_PUMP_H__
#define __WATER_PUMP_H__

#include "board.h"

void water_pump_init(void);
void water_pump_on(void);
void water_pump_off(void);
uint8_t water_pump_get_status(void);

#endif
