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
#include "usart.h"
#include "dht11.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* 上位机命令位定义（数据低 4 位有效） */
#define CMD_LED_RED    0x01U   /* bit0：红灯 PA7，1 点亮 / 0 熄灭 */
#define CMD_LED_BLUE   0x02U   /* bit1：蓝灯 PA6，1 点亮 / 0 熄灭 */
#define CMD_BUZZER     0x04U   /* bit2：蜂鸣器 PA5，1 响 / 0 不响 */
#define CMD_REPORT_DHT 0x08U   /* bit3：1 = 回发一次温湿度 / 0 = 不回发 */

/* 红灯/蓝灯/蜂鸣器均为低电平有效：拉低 = 点亮/响，拉高 = 熄灭/不响 */
#define ACTIVE(bit)  ((bit) ? GPIO_PIN_RESET : GPIO_PIN_SET)

/**
  * @brief  把 0~255 的整数按十进制追加到缓冲区
  * @retval 新的写入位置
  */
static uint8_t *AppendDec(uint8_t *p, uint8_t v)
{
  if (v >= 100U) { *p++ = (uint8_t)('0' + (v / 100U)); }
  if (v >= 10U)  { *p++ = (uint8_t)('0' + ((v / 10U) % 10U)); }
  *p++ = (uint8_t)('0' + (v % 10U));

  return p;
}

/**
  * @brief  把温湿度按 "T=26,H=58\r\n" 的格式发到串口
  * @note   上位机解析：去掉结尾的 \r\n，先按 ',' 分成两段，再各按 '=' 取数值
  * @param  ret  0 表示读取成功，其余为失败（此时发 T=ERR,H=ERR）
  * @param  temp 温度整数（℃）
  * @param  humi 湿度整数（%RH）
  */
static void Send_DHT11(uint8_t ret, uint8_t temp, uint8_t humi)
{
  uint8_t buf[24];
  uint8_t *p = buf;
  const char *s;

  if (ret == 0U)
  {
    *p++ = 'T'; *p++ = '=';
    p = AppendDec(p, temp);
    *p++ = ','; *p++ = 'H'; *p++ = '=';
    p = AppendDec(p, humi);
  }
  else
  {
    for (s = "T=ERR,H=ERR"; *s != '\0'; s++) { *p++ = (uint8_t)*s; }
  }

  *p++ = '\r';
  *p++ = '\n';

  HAL_UART_Transmit(&huart1, buf, (uint16_t)(p - buf), 100U);
}

/**
  * @brief  解析并执行一条命令
  * @param  cmd 上位机下发的原始数据，仅低 4 位有效
  */
static void Execute_Command(uint8_t cmd)
{
  cmd &= 0x0FU;

  /* bit0~bit2：红灯 / 蓝灯 / 蜂鸣器 */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, ACTIVE(cmd & CMD_LED_RED));
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, ACTIVE(cmd & CMD_LED_BLUE));
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, ACTIVE(cmd & CMD_BUZZER));

  /* bit3 为 1：读一次 DHT11 并把温湿度回发给上位机 */
  if ((cmd & CMD_REPORT_DHT) != 0U)
  {
    uint8_t temp = 0U;
    uint8_t humi = 0U;
    uint8_t ret = DHT11_Read_Data(&temp, &humi);

    Send_DHT11(ret, temp, humi);
  }
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* 初始化 DHT11（PB12），并把探测结果发到串口 */
  {
    const char *msg = (DHT11_Init() == 0U) ? "DHT11 init ok\r\n" : "DHT11 init fail\r\n";
    const char *s;
    uint8_t buf[20];
    uint8_t *p = buf;

    for (s = msg; *s != '\0'; s++) { *p++ = (uint8_t)*s; }
    HAL_UART_Transmit(&huart1, buf, (uint16_t)(p - buf), 100U);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint8_t cmd;

    /* 轮询接收队列，有数据就取出并执行 */
    if (Usart_Queue_Pop(&cmd))
    {
      Execute_Command(cmd);
    }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
