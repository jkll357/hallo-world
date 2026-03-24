/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "command.h"

/**
 * @brief 串口相关定义和变量
 */

/**
 * @brief 串口接收缓冲区大小
 */
#define UART_RX_BUFFER_SIZE 256

/**
 * @brief DMA接收缓冲区
 * 
 * 用于DMA直接访问，存储接收到的数据
 * 不需要volatile，因为DMA硬件直接访问内存
 */
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];

/**
 * @brief 接收状态标志和数据长度
 */
volatile uint8_t uart_rx_flag = 0;      // 接收完成标志，由IDLE中断设置
volatile uint16_t uart_rx_count = 0;    // 接收数据计数

/**
 * @brief 用户数据缓冲区
 * 
 * 用于主循环处理，存储从DMA缓冲区复制的数据
 */
volatile uint8_t uart_user_buffer[UART_RX_BUFFER_SIZE];
volatile uint16_t uart_user_len = 0;     // 用户数据长度

/**
 * @brief 命令处理相关变量
 */
static uint8_t rx_buffer[UART_RX_BUFFER_SIZE];  // 命令缓冲区
static uint16_t rx_index = 0;                  // 命令缓冲区索引

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* USER CODE BEGIN USART1_Init 2 */
  /* 注意：不要在初始化时启用RXNE中断，与DMA接收冲突 */
  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief 初始化 DMA 接收中断模式
  * @retval None
  * 
  * 使用 IDLE 中断 + DMA 循环模式实现不定长数据接收
  * 1. 清除可能存在的IDLE标志位
  * 2. 启用 IDLE 中断
  * 3. 启动 DMA 循环接收
  */
void UART_DMA_Receive_IT_Init(void)
{
  /* 1. 清除可能存在的IDLE标志位（使用HAL库宏） */
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);

  /* 2. 启用 IDLE 中断 */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

  /* 3. 启动 DMA 循环接收 */
  if(HAL_UART_Receive_DMA(&huart1, uart_rx_buffer, UART_RX_BUFFER_SIZE) != HAL_OK)
  {
      Error_Handler();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Channel5;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;  // 提高优先级
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);  // 适当降低优先级
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief 发送单个字符
  * @param ch: 要发送的字符
  * @retval 发送的字符
  */
int UART_PutChar(char ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
  * @brief 简单的字符串输出函数
  * @param str: 要输出的字符串
  * @retval 输出的字符数
  */
int UART_PutString(const char *str)
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    
    if (len > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)str, len, HAL_MAX_DELAY);
    }
    
    return len;
}

/**
  * @brief 错误日志输出函数
  * @param format: 错误信息
  * @retval 输出的字符数
  */
int UART_LogError(const char *format)
{
    int len = 0;
    len += UART_PutString("[ERROR] ");
    len += UART_PutString(format);
    len += UART_PutString("\r\n");
    return len;
}

/**
  * @brief 重定向 _write 函数，使 printf 能够通过 USART 输出
  * @param file: 文件描述符
  * @param ptr: 数据指针
  * @param len: 数据长度
  * @retval 实际写入的长度
  */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/**
  * @brief 通过 DMA 发送数据（非阻塞模式）
  * @param pData: 发送数据指针
  * @param Size: 数据长度
  * @retval HAL 状态
  */
HAL_StatusTypeDef UART_SendData(uint8_t *pData, uint16_t Size)
{
    // 等待上一次发送完成（带超时）
    uint32_t tickstart = HAL_GetTick();
    while (huart1.gState != HAL_UART_STATE_READY)
    {
        if ((HAL_GetTick() - tickstart) > 100)
        {
            // 超时：强制终止当前传输
            HAL_UART_AbortTransmit(&huart1);
            break;
        }
    }

    // 启动新的发送
    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart1, pData, Size);
    
    // 等待发送完成（带超时）
    tickstart = HAL_GetTick();
    while (huart1.gState != HAL_UART_STATE_READY)
    {
        if ((HAL_GetTick() - tickstart) > 100)
        {
            // 超时：强制终止当前传输
            HAL_UART_AbortTransmit(&huart1);
            break;
        }
    }
    
    return status;
}

/**
  * @brief 检查是否有新数据接收
  * @retval 0: 无新数据, 1: 有新数据
  */
uint8_t UART_IsDataReceived(void)
{
    return uart_rx_flag;
}

/**
  * @brief 获取接收到的数据
  * @param buffer: 数据缓冲区
  * @param max_len: 最大长度
  * @retval 实际接收到的数据长度
  */
uint16_t UART_GetReceivedData(uint8_t *buffer, uint16_t max_len)
{
    uint16_t len = 0;
    
    if (uart_rx_flag)
    {
        // 复制数据到用户缓冲区
        len = (uart_user_len < max_len) ? uart_user_len : max_len;
        memcpy(buffer, (void*)uart_user_buffer, len);
        
        // 清除标志
        uart_rx_flag = 0;
    }
    
    return len;
}

/**
  * @brief 处理IDLE中断
  * @retval None
  * @note 此函数在USART1_IRQHandler中调用，只设置标志，不执行耗时操作
  * 
  * 当检测到串口空闲时（数据传输结束），计算接收长度并设置标志
  * 1. 计算接收到的数据长度
  * 2. 重置DMA接收（停止后重新启动）
  * 3. 复制数据到用户缓冲区
  * 4. 设置标志位，通知主循环处理
  */
void UART_HandleIdleInterrupt(void)
{
    // 计算接收到的数据长度：缓冲区大小减去DMA当前计数器值
    uint16_t recv_len = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
    
    // 重置DMA接收（停止后重新启动），确保持续接收
    HAL_UART_DMAStop(&huart1);
    HAL_UART_Receive_DMA(&huart1, uart_rx_buffer, UART_RX_BUFFER_SIZE);
    
    if (recv_len > 0 && !uart_rx_flag)  // 确保上一次数据已被处理
    {
        // 复制数据到用户缓冲区，避免在中断中处理耗时操作
        uint16_t copy_len = (recv_len < UART_RX_BUFFER_SIZE) ? recv_len : UART_RX_BUFFER_SIZE;
        memcpy((void*)uart_user_buffer, uart_rx_buffer, copy_len);
        uart_user_len = copy_len;
        
        // 设置标志位，通知主循环处理
        uart_rx_flag = 1;
        uart_rx_count = copy_len;
    }
}

/**
  * @brief 获取DMA接收缓冲区指针（用于调试）
  * @retval 缓冲区指针
  */
uint8_t* UART_GetRxBufferPtr(void)
{
    return uart_rx_buffer;
}

/**
  * @brief 获取当前DMA计数器值（用于调试）
  * @retval 计数器值
  */
uint16_t UART_GetRxCounter(void)
{
    return __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
}

/**
  * @brief 串口接收回调函数 - 符合HAL库标准
  * @param huart UART句柄
  * 
  * 处理接收到的数据，解析命令并执行
  * 1. 检查是否是USART1
  * 2. 从用户缓冲区获取数据和长度
  * 3. 解析数据，检测命令结束符
  * 4. 处理完整的命令
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        // 从用户缓冲区获取数据和长度
        volatile uint8_t *data = uart_user_buffer;
        uint16_t size = uart_user_len;
        
        for (uint16_t i = 0; i < size; i++)
        {
            if (data[i] == '\r' || data[i] == '\n')
            {
                // 命令结束，处理命令
                if (rx_index > 0)
                {
                    process_command(rx_buffer, rx_index);
                    rx_index = 0;  // 重置命令缓冲区索引
                }
            }
            else if (rx_index < UART_RX_BUFFER_SIZE - 1)
            {
                // 存储接收到的字符到命令缓冲区
                rx_buffer[rx_index++] = data[i];
            }
        }
    }
}

/* USER CODE END 1 */
