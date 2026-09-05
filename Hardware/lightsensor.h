#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

#include "stm32f1xx_hal.h"

#define LIGHTSENSOR_PIN   GPIO_PIN_13
#define LIGHTSENSOR_PORT  GPIOB

void LightSensor_Init(void);
uint8_t LightSensor_Get(void);

#endif
