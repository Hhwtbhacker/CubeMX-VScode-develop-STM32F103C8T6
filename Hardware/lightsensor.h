#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

#include "stm32f1xx_hal.h"

/* 数字输出 DO：接 PB13 */
#define LIGHTSENSOR_PIN   GPIO_PIN_13
#define LIGHTSENSOR_PORT  GPIOB

/* 模拟输出 AO：接 PA0（ADC1_IN0） */
#define LIGHTSENSOR_AO_PIN   GPIO_PIN_0
#define LIGHTSENSOR_AO_PORT  GPIOA

void LightSensor_Init(void);
uint8_t LightSensor_Get(void);       /* 数字量：0=暗，1=亮 */
uint16_t LightSensor_ReadADC(void);  /* 模拟量原始值 0~4095 */
float LightSensor_GetLux(void);      /* 光照强度，单位 lux */

#endif
