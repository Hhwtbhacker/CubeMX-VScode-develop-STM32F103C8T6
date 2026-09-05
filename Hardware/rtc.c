#include "rtc.h"

/* 初始时间（仅在备份域首次上电、时间尚未初始化时写入一次） */
#define RTC_INIT_HOUR   17U
#define RTC_INIT_MIN    23U
#define RTC_INIT_SEC    0U

/* 备份域标志：用于判断 RTC 是否已初始化，避免每次复位都重设时间 */
#define RTC_MAGIC_FLAG  0x5A5AU

/* LSE = 32768Hz，预分频值 32767 → 计数器每秒 +1 */
#define RTC_PRESCALER   32767U

/* 进入 RTC 配置模式（写 PRL/CNT 前必须） */
static void RTC_EnterConfigMode(void)
{
    while ((RTC->CRL & RTC_CRL_RTOFF) == 0U) { }
    RTC->CRL |= RTC_CRL_CNF;
}

/* 退出 RTC 配置模式 */
static void RTC_ExitConfigMode(void)
{
    RTC->CRL &= ~RTC_CRL_CNF;
    while ((RTC->CRL & RTC_CRL_RTOFF) == 0U) { }
}

/* 等待 RTC 影子寄存器与 APB1 时钟同步（读 CNT 前必须） */
static void RTC_WaitSync(void)
{
    RTC->CRL &= ~RTC_CRL_RSF;
    while ((RTC->CRL & RTC_CRL_RSF) == 0U) { }
}

static uint32_t RTC_GetCounter(void)
{
    uint32_t counter;
    RTC_WaitSync();
    /* 必须先读高 16 位再读低 16 位 */
    counter = ((uint32_t)RTC->CNTH << 16) | RTC->CNTL;
    return counter;
}

static void RTC_SetCounter(uint32_t counter)
{
    RTC_EnterConfigMode();
    RTC->CNTH = (uint16_t)(counter >> 16);
    RTC->CNTL = (uint16_t)(counter & 0xFFFFU);
    RTC_ExitConfigMode();
}

void RTC_Init(void)
{
    /* 使能电源与备份域接口时钟 */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();

    /* 允许访问备份域寄存器 */
    HAL_PWR_EnableBkUpAccess();

    /* RTC 一旦使能只能通过备份域复位关闭，所以仅在其未使能时配置一次 */
    if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0U)
    {
        /* 启动 LSE 并等待就绪 */
        __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
        while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) { }

        /* 选择 LSE 为 RTC 时钟源并使能 RTC */
        __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
        __HAL_RCC_RTC_ENABLE();

        /* 配置 1Hz 预分频 */
        RTC_EnterConfigMode();
        RTC->PRLH = (uint16_t)(RTC_PRESCALER >> 16);
        RTC->PRLL = (uint16_t)(RTC_PRESCALER & 0xFFFFU);
        RTC_ExitConfigMode();
    }

    /* 首次上电（备份域标志未置位）时写入初始时间 */
    if (BKP->DR1 != RTC_MAGIC_FLAG)
    {
        RTC_SetTime(RTC_INIT_HOUR, RTC_INIT_MIN, RTC_INIT_SEC);
        BKP->DR1 = RTC_MAGIC_FLAG;
    }
}

void RTC_SetTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    uint32_t seconds;

    if (hour > 23U)   { hour = 23U; }
    if (minute > 59U) { minute = 59U; }
    if (second > 59U) { second = 59U; }

    seconds = (uint32_t)hour * 3600U + (uint32_t)minute * 60U + (uint32_t)second;
    RTC_SetCounter(seconds);
}

void RTC_GetTime(uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    /* 计数器按“一天内的秒数”计（0~86399），取模实现每天回绕 */
    uint32_t seconds = RTC_GetCounter() % 86400U;

    *hour   = (uint8_t)(seconds / 3600U);
    *minute = (uint8_t)((seconds % 3600U) / 60U);
    *second = (uint8_t)(seconds % 60U);
}
