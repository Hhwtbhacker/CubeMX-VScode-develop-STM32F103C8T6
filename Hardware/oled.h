#ifndef __OLED_H
#define __OLED_H

#include "stm32f1xx_hal.h"

// PB8 SCL  PB9 SDA
#define OLED_SCL_PIN   GPIO_PIN_8
#define OLED_SCL_PORT  GPIOB
#define OLED_SDA_PIN   GPIO_PIN_9
#define OLED_SDA_PORT  GPIOB

#define OLED_SCL_H()   HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_SET)
#define OLED_SCL_L()   HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_RESET)
#define OLED_SDA_H()   HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_SET)
#define OLED_SDA_L()   HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_RESET)

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowString(uint8_t x, uint8_t y, char *str);

#endif
