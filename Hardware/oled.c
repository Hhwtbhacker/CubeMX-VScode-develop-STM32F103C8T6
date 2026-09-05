#include "oled.h"
#include "oled_font.h"

/* SCL=PB8, SDA=PB9，软件 I2C，开漏输出，依赖模块上的 4.7k 上拉电阻 */
#define OLED_SCL_PIN    GPIO_PIN_8
#define OLED_SDA_PIN    GPIO_PIN_9
#define OLED_SCL(x)     HAL_GPIO_WritePin(GPIOB, OLED_SCL_PIN, (GPIO_PinState)(x))
#define OLED_SDA(x)     HAL_GPIO_WritePin(GPIOB, OLED_SDA_PIN, (GPIO_PinState)(x))

static uint8_t OLED_Addr = 0x78;

/* 微秒级延时：基于 DWT 周期计数器，精确且不受主频优化影响 */
static void OLED_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

static void OLED_I2C_Delay(void)
{
    OLED_DelayUs(2);    /* 2us，SSD1306 I2C 最高 400kHz 对应的最小位周期附近，可调 1~10 */
}

static void OLED_I2C_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 使能 DWT 周期计数器，供微秒延时使用 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = OLED_SCL_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OLED_SDA_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    OLED_SCL(1);
    OLED_SDA(1);
}

static void OLED_I2C_Start(void)
{
    OLED_SDA(1);
    OLED_I2C_Delay();
    OLED_SCL(1);
    OLED_I2C_Delay();
    OLED_SDA(0);
    OLED_I2C_Delay();
    OLED_SCL(0);
    OLED_I2C_Delay();
}

static void OLED_I2C_Stop(void)
{
    OLED_SDA(0);
    OLED_I2C_Delay();
    OLED_SCL(1);
    OLED_I2C_Delay();
    OLED_SDA(1);
    OLED_I2C_Delay();
}

static uint8_t OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLED_SDA(!!(Byte & (0x80 >> i)));
        OLED_I2C_Delay();
        OLED_SCL(1);
        OLED_I2C_Delay();
        OLED_SCL(0);
    }

    OLED_SDA(1);
    OLED_I2C_Delay();
    OLED_SCL(1);
    OLED_I2C_Delay();
    uint8_t ack = (GPIOB->IDR & OLED_SDA_PIN) ? 1u : 0u;
    OLED_SCL(0);
    return ack;
}

static void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_Addr);
    OLED_I2C_SendByte(0x00);
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

static void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_Addr);
    OLED_I2C_SendByte(0x40);
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}

static void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
    OLED_WriteCommand(0x00 | (X & 0x0F));
}

void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for (i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
    }
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

static void OLED_DetectAddr(void)
{
    OLED_Addr = 0x78;

    OLED_I2C_Start();
    if (OLED_I2C_SendByte(0x78) == 0)
    {
        OLED_I2C_Stop();
        return;
    }
    OLED_I2C_Stop();

    OLED_I2C_Start();
    if (OLED_I2C_SendByte(0x7A) == 0)
    {
        OLED_Addr = 0x7A;
    }
    OLED_I2C_Stop();
}

void OLED_Init(void)
{
    OLED_I2C_Init();
    HAL_Delay(100);
    OLED_DetectAddr();

    OLED_WriteCommand(0xAE);
    OLED_WriteCommand(0xD5); OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3); OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0xDA); OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9); OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB); OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0x8D); OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);

    OLED_Clear();
}
