#include "lightsensor.h"
#include <math.h>

/* GL5528 光敏电阻参数（可实测后微调） */
#define GL5528_R10_OHM    10000.0f   /* 10 lux 时的阻值，典型 10kΩ */
#define GL5528_GAMMA      0.6f       /* γ 系数，典型 0.6 */
#define LDR_SERIES_OHM    10000.0f   /* 分压固定电阻（VCC↔AO），10kΩ */

/* 配置 ADC1，用 PA0（通道 0）读取光敏模拟量 */
static void LightSensor_ADC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PA0 模拟输入 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = LIGHTSENSOR_AO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(LIGHTSENSOR_AO_PORT, &GPIO_InitStruct);

    /* 使能 ADC1 时钟 */
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* ADC 时钟 = PCLK2 / 6 = 72 / 6 = 12MHz（F103 ADC 最高 14MHz） */
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;

    /* ADC1 上电并稳定 */
    ADC1->CR2 |= ADC_CR2_ADON;
    HAL_Delay(2);

    /* 校准 */
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL) { }
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL) { }

    /* 通道 0 采样时间：239.5 周期，最稳定 */
    ADC1->SMPR2 = (7U << ADC_SMPR2_SMP0_Pos);

    /* 规则序列：1 个转换，通道 0 */
    ADC1->SQR1 = 0;                 /* L[3:0]=0 → 转换 1 个通道 */
    ADC1->SQR3 = 0;                 /* SQ1 = 通道 0 */

    /* 软件触发单次转换 */
    ADC1->CR2 |= ADC_CR2_EXTSEL;    /* EXTSEL=111 → SWSTART 软件触发 */
    ADC1->CR2 |= ADC_CR2_EXTTRIG;   /* 使能规则组外部/软件触发 */
}

void LightSensor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 数字输出 DO：PB13 上拉输入 */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = LIGHTSENSOR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(LIGHTSENSOR_PORT, &GPIO_InitStruct);

    /* 模拟输出 AO：初始化 ADC */
    LightSensor_ADC_Init();
}

uint8_t LightSensor_Get(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(LIGHTSENSOR_PORT, LIGHTSENSOR_PIN);
}

uint16_t LightSensor_ReadADC(void)
{
    /* 软件触发一次转换，等 EOC */
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while ((ADC1->SR & ADC_SR_EOC) == 0U) { }
    return (uint16_t)(ADC1->DR & 0x0FFFU);
}

float LightSensor_GetLux(void)
{
    uint16_t adc = LightSensor_ReadADC();

    /* 夹紧边界，避免除零：全暗 adc≈4095，全亮 adc≈0 */
    if (adc >= 4095U) { adc = 4094U; }
    if (adc == 0U)    { adc = 1U; }

    /*
     * 分压电路：10kΩ 上拉 VCC↔AO，LDR 接 AO↔GND
     *   Vout = VCC * R_ldr / (R_ser + R_ldr)
     *   adc/4095 = R_ldr / (R_ser + R_ldr)
     *   → R_ldr = R_ser * adc / (4095 - adc)
     */
    float r_ldr = LDR_SERIES_OHM * (float)adc / (float)(4095U - adc);

    /*
     * GL5528 阻值-照度关系：R = R10 * (10/lux)^γ
     *   → lux = 10 * (R10 / R)^(1/γ)
     */
    float lux = 10.0f * powf(GL5528_R10_OHM / r_ldr, 1.0f / GL5528_GAMMA);

    return lux;
}
