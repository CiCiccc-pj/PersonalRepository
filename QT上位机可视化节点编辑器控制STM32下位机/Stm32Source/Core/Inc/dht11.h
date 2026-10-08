/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dht11.h
  * @brief   DHT11 温湿度传感器驱动（单总线）
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DHT11_H__
#define __DHT11_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* DHT11 数据线（DATA）所接的引脚，改这里即可换引脚 */
#define DHT11_GPIO_PORT   GPIOB
#define DHT11_GPIO_PIN    GPIO_PIN_12

/* USER CODE BEGIN Prototypes */

/**
  * @brief  初始化 DHT11：配置数据线并探测传感器是否存在
  * @note   内部会等待 1s（DHT11 上电后需 >1s 稳定），需已初始化 SysTick（HAL_Init）
  * @retval 0 检测到 DHT11；1 无响应
  */
uint8_t DHT11_Init(void);

/**
  * @brief  读取一次温湿度（DHT11 两次读取间隔需 >1s）
  * @param  temp 输出：温度整数部分（℃）
  * @param  humi 输出：湿度整数部分（%RH）
  * @retval 0 成功；1 无响应；2 校验和错误
  */
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __DHT11_H__ */
