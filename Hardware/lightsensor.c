#include "lightsensor.h"

void LightSensor_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LIGHTSENSOR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(LIGHTSENSOR_PORT, &GPIO_InitStruct);
}

uint8_t LightSensor_Get(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(LIGHTSENSOR_PORT, LIGHTSENSOR_PIN);
}
