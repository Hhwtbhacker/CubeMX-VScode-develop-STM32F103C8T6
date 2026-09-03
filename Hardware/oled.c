#include "oled.h"
#include "oled_font.h"

static void OLED_I2C_Delay(void)
{
    HAL_Delay(1);
}

static void I2C_Start(void)
{
    OLED_SDA_H();
    OLED_SCL_H();
    OLED_I2C_Delay();
    OLED_SDA_L();
    OLED_I2C_Delay();
    OLED_SCL_L();
}

static void I2C_Stop(void)
{
    OLED_SDA_L();
    OLED_SCL_H();
    OLED_I2C_Delay();
    OLED_SDA_H();
}

static void I2C_SendByte(uint8_t dat)
{
    uint8_t i;
    for(i = 0; i < 8; i++)
    {
        OLED_SCL_L();
        if(dat & 0x80)
            OLED_SDA_H();
        else
            OLED_SDA_L();
        dat <<= 1;
        OLED_I2C_Delay();
        OLED_SCL_H();
        OLED_I2C_Delay();
    }
    OLED_SCL_L();
}

static void OLED_WriteCmd(uint8_t cmd)
{
    I2C_Start();
    I2C_SendByte(0x7A);
    I2C_SendByte(0x00);
    I2C_SendByte(cmd);
    I2C_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    I2C_Start();
    I2C_SendByte(0x7A);
    I2C_SendByte(0x40);
    I2C_SendByte(data);
    I2C_Stop();
}

void OLED_Init(void)
{
    HAL_Delay(100);
    OLED_WriteCmd(0xAE);
    OLED_WriteCmd(0xD5);
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8);
    OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xD3);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x8D);
    OLED_WriteCmd(0x14);
    OLED_WriteCmd(0x20);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0xA1);
    OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0xDA);
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81);
    OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9);
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB);
    OLED_WriteCmd(0x30);
    OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0xAF);
    OLED_Clear();
}

void OLED_Clear(void)
{
    uint8_t y, x;
    for(y = 0; y < 8; y++)
    {
        OLED_WriteCmd(0xb0 + y);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for(x = 0; x < 128; x++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, char *str)
{
    uint8_t c = 0;
    while(*str != '\0')
    {
        if((*str >= ' ') && (*str <= '~'))
        {
            c = *str - ' ';
            OLED_WriteCmd(0xb0 + y);
            OLED_WriteCmd(x % 16);
            OLED_WriteCmd((x / 16) | 0x10);
            for(uint8_t i = 0; i < 8; i++)
            {
                OLED_WriteData(F8X16[c*16 + i]);
            }
            OLED_WriteCmd(0xb0 + y + 1);
            OLED_WriteCmd(x % 16);
            OLED_WriteCmd((x / 16) | 0x10);
            for(uint8_t i = 0; i < 8; i++)
            {
                OLED_WriteData(F8X16[c*16 + i + 8]);
            }
            x += 8;
            if(x > 120)
            {
                x = 0;
                y += 2;
            }
        }
        str++;
    }
}
