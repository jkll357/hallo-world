/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private defines */
#define UART_RX_BUFFER_SIZE 256

/* 外部声明接收状态变量 */
extern volatile uint8_t uart_rx_flag;
extern volatile uint16_t uart_rx_count;
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/**
  * @brief 发送单个字符
  * @param ch: 要发送的字符
  * @retval 发送的字符
  */
int UART_PutChar(char ch);

/**
  * @brief 简单的字符串输出函数
  * @param str: 要输出的字符串
  * @retval 输出的字符数
  */
int UART_PutString(const char *str);

/**
  * @brief 错误日志输出函数
  * @param format: 错误信息
  * @retval 输出的字符数
  */
int UART_LogError(const char *format);

/**
  * @brief 初始化 DMA 接收中断模式
  */
void UART_DMA_Receive_IT_Init(void);

/**
  * @brief 通过 DMA 发送数据（非阻塞模式）
  * @param pData: 发送数据指针
  * @param Size: 数据长度
  * * @retval HAL 状态
  */
HAL_StatusTypeDef UART_SendData(uint8_t *pData, uint16_t Size);

/**
  * @brief 检查是否有新数据接收
  * @retval 0: 无新数据, 1: 有新数据
  */
uint8_t UART_IsDataReceived(void);

/**
  * @brief 获取接收到的数据
  * @param buffer: 数据缓冲区
  * @param max_len: 最大长度
  * @retval 实际接收到的数据长度
  */
uint16_t UART_GetReceivedData(uint8_t *buffer, uint16_t max_len);

/**
  * @brief 处理IDLE中断 - 在中断中调用
  */
void UART_HandleIdleInterrupt(void);

/**
  * @brief 获取DMA接收缓冲区指针（用于调试）
  * @retval 缓冲区指针
  */
uint8_t* UART_GetRxBufferPtr(void);

/**
  * @brief 获取当前DMA计数器值（用于调试）
  * @retval 计数器值
  */
uint16_t UART_GetRxCounter(void);

void UART_RxCallback(volatile uint8_t *data, uint16_t size);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
