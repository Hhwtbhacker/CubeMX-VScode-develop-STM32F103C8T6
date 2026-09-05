#ifndef __RTC_H
#define __RTC_H

#include "stm32f1xx_hal.h"

void RTC_Init(void);
void RTC_SetTime(uint8_t hour, uint8_t minute, uint8_t second);
void RTC_GetTime(uint8_t *hour, uint8_t *minute, uint8_t *second);

#endif
