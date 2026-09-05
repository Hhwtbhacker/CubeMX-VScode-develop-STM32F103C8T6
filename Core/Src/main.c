/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "lightsensor.h"
#include "rtc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 把当前时间格式化为 "HH:MM:SS" 显示在指定行/列 */
static void OLED_ShowTime(uint8_t Line, uint8_t Column)
{
    uint8_t hour, minute, second;
    char buf[9];

    RTC_GetTime(&hour, &minute, &second);

    buf[0] = (char)('0' + hour / 10);
    buf[1] = (char)('0' + hour % 10);
    buf[2] = ':';
    buf[3] = (char)('0' + minute / 10);
    buf[4] = (char)('0' + minute % 10);
    buf[5] = ':';
    buf[6] = (char)('0' + second / 10);
    buf[7] = (char)('0' + second % 10);
    buf[8] = '\0';

    OLED_ShowString(Line, Column, buf);
}

/* 光敏消抖：连续 count 次读到的值都等于 target 才算数 */
static uint8_t LightSensor_Debounce(uint8_t target, uint16_t count, uint16_t interval_ms)
{
    for (uint16_t i = 0; i < count; i++)
    {
        HAL_Delay(interval_ms);
        if (LightSensor_Get() != target)
        {
            return 0;
        }
    }
    return 1;
}

/* 在第 3 行显示状态文字：先用空格清空整行，避免长短文字切换时字符残留重叠 */
static void OLED_ShowStatus(const char *msg)
{
    OLED_ShowString(3, 1, "                ");   /* 16 个空格，清空第 3 行 */
    OLED_ShowString(3, 4, (char *)msg);         /* 第 4 列开始显示 */
}

/* 在第 4 行显示光照强度：Lux: xxxxx（单位 lux，固定宽度避免残留） */
static void OLED_ShowLux(void)
{
    uint32_t lux = (uint32_t)LightSensor_GetLux();
    char buf[15];

    if (lux > 99999U) { lux = 99999U; }   /* 截断到 5 位 */

    buf[0] = 'L';
    buf[1] = 'u';
    buf[2] = 'x';
    buf[3] = ':';
    buf[4] = ' ';
    buf[5] = (char)('0' + (lux / 10000) % 10);
    buf[6] = (char)('0' + (lux / 1000) % 10);
    buf[7] = (char)('0' + (lux / 100) % 10);
    buf[8] = (char)('0' + (lux / 10) % 10);
    buf[9] = (char)('0' + lux % 10);
    buf[10] = ' ';
    buf[11] = 'l';
    buf[12] = 'x';
    buf[13] = ' ';
    buf[14] = '\0';

    OLED_ShowString(4, 1, buf);   /* 第 4 行第 1 列，长度固定无残留 */
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  LightSensor_Init();
  RTC_Init();
  OLED_ShowString(1, 3, "Current Time");
  uint8_t last_second = 0xFF;   // 记录上次显示的秒，秒变化时才刷新
  uint8_t last_status = 1;      // 光敏状态：1=亮，0=暗
  uint32_t last_lux_tick = 0;   // 上次刷新光照强度的时间
  OLED_ShowLux();               // 先显示一次光照强度
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    uint8_t hour, minute, second;
    uint8_t sensor = LightSensor_Get();

    RTC_GetTime(&hour, &minute, &second);

    if (second != last_second)
    {
      OLED_ShowTime(2, 5);   // 第 2 行居中显示 HH:MM:SS
      last_second = second;
    }

    /* 每 500ms 刷新一次第 4 行的光照强度 */
    if (HAL_GetTick() - last_lux_tick >= 500)
    {
      OLED_ShowLux();
      last_lux_tick = HAL_GetTick();
    }

    /* 光线变暗（被遮挡）→ 第 3 行显示 Welcome */
    if (sensor == 0 && last_status == 1)
    {
      if (LightSensor_Debounce(0, 2, 250))
      {
        OLED_ShowStatus(" Welcome");
        last_status = 0;
      }
    }
    /* 光线变亮 → 第 3 行显示 Bye（补空格清除 Welcome 的残留字符） */
    else if (sensor == 1 && last_status == 0)
    {
      if (LightSensor_Debounce(1, 4, 500))
      {
        OLED_ShowStatus("   Bye");
        last_status = 1;
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
