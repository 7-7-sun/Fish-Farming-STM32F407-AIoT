#ifndef __HEATING_ROD_H__
#define __HEATING_ROD_H__

#include "board.h"

void heating_rod_init(void);
void heating_rod_on(void);
void heating_rod_off(void);
uint8_t heating_rod_get_status(void);

#endif
