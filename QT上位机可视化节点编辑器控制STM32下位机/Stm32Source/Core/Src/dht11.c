/**
  ******************************************************************************
  * @file    dht11.c
  * @brief   DHT11 温湿度传感器驱动（单总线，默认 PB12）
  *
  * 时序要点（DHT11 数据手册）：
  *   1. 主机拉低数据线 >=18ms 作为起始信号，再拉高 20~40us 释放；
  *   2. DHT11 响应：先拉低 80us，再拉高 80us；
  *   3. 之后每位数据：50us 低电平 + 高电平，高电平 26~28us 表示 0，70us 表示 1；
  *   4. 共 40 位 = 湿度整数 + 湿度小数 + 温度整数 + 温度小数 + 校验和，
  *      校验和 = 前 4 字节之和的低 8 位。
  *
  * 注意：HAL_Delay 只有毫秒级，因此这里用 Cortex-M3 的 DWT 周期计数器做 us 级延时。
  ******************************************************************************
  */
#include "dht11.h"

/* 数据线电平操作（输出用） */
#define DHT11_DQ_OUT(level) \
  HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, (level) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/* 读取数据线电平（输入用） */
#define DHT11_DQ_IN \
  (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)

/* USER CODE BEGIN 0 */

/**
  * @brief  使能 DWT 周期计数器，作为微秒延时的时间基准
  */
static void DWT_Delay_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  微秒级忙等延时
  * @param  us 延时长度（微秒）
  */
static void delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (SystemCoreClock / 1000000U);

  /* CYCCNT 为 32 位会回绕，用无符号相减判断即可 */
  while ((DWT->CYCCNT - start) < ticks)
  {
  }
}

/**
  * @brief  把数据线切换为推挽输出
  */
static void DHT11_IO_Out(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin   = DHT11_GPIO_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  把数据线切换为输入（内部上拉，保证总线空闲时为高电平）
  * @note   模块自带 4.7k~10k 上拉最好；裸传感器需外接上拉电阻
  */
static void DHT11_IO_In(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin  = DHT11_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  主机发送起始信号：拉低 20ms，再拉高 30us
  */
static void DHT11_Rst(void)
{
  DHT11_IO_Out();
  DHT11_DQ_OUT(0);      /* 拉低数据线 */
  HAL_Delay(20);        /* >=18ms */
  DHT11_DQ_OUT(1);      /* 释放总线 */
  delay_us(30);         /* 主机拉高 20~40us */
}

/**
  * @brief  等待 DHT11 的响应信号
  * @retval 0 有响应；1 无响应
  */
static uint8_t DHT11_Check(void)
{
  uint8_t retry = 0U;

  DHT11_IO_In();

  /* DHT11 会拉低 40~80us */
  while (DHT11_DQ_IN && (retry < 100U))
  {
    retry++;
    delay_us(1);
  }
  if (retry >= 100U)
  {
    return 1U;
  }

  retry = 0U;
  /* 随后再拉高 40~80us */
  while ((!DHT11_DQ_IN) && (retry < 100U))
  {
    retry++;
    delay_us(1);
  }
  if (retry >= 100U)
  {
    return 1U;
  }

  return 0U;
}

/**
  * @brief  从 DHT11 读出一位
  * @retval 0 或 1
  */
static uint8_t DHT11_Read_Bit(void)
{
  uint8_t retry = 0U;

  /* 等 50us 的低电平结束 */
  while (DHT11_DQ_IN && (retry < 100U))
  {
    retry++;
    delay_us(1);
  }

  retry = 0U;
  /* 等高电平开始 */
  while ((!DHT11_DQ_IN) && (retry < 100U))
  {
    retry++;
    delay_us(1);
  }

  /* 高电平持续 >40us 判为 1，否则为 0 */
  delay_us(40);
  return DHT11_DQ_IN ? 1U : 0U;
}

/**
  * @brief  从 DHT11 读出一个字节（高位在前）
  */
static uint8_t DHT11_Read_Byte(void)
{
  uint8_t i;
  uint8_t dat = 0U;

  for (i = 0U; i < 8U; i++)
  {
    dat <<= 1;
    dat |= DHT11_Read_Bit();
  }

  return dat;
}

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
  uint8_t buf[5];
  uint8_t i;

  DHT11_Rst();

  if (DHT11_Check() != 0U)
  {
    return 1U;   /* 传感器没响应 */
  }

  for (i = 0U; i < 5U; i++)
  {
    buf[i] = DHT11_Read_Byte();
  }

  /* 校验：前 4 字节之和的低 8 位 == 第 5 字节 */
  if ((uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]) != buf[4])
  {
    return 2U;
  }

  *humi = buf[0];   /* 湿度整数部分 */
  *temp = buf[2];   /* 温度整数部分 */

  return 0U;
}

uint8_t DHT11_Init(void)
{
  DHT11_IO_Out();
  DHT11_DQ_OUT(1);        /* 空闲保持高电平 */
  DWT_Delay_Init();

  HAL_Delay(1000);        /* DHT11 上电后需 >1s 越过不稳定状态 */

  DHT11_Rst();

  return (DHT11_Check() == 0U) ? 0U : 1U;
}

/* USER CODE END 1 */
