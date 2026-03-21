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
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>

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
// 发送缓冲区
static uint8_t tx_buffer[UART_RX_BUFFER_SIZE];

// 接收缓冲区
static uint8_t rx_buffer[UART_RX_BUFFER_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  // 初始化 DMA 接收中断模式（支持不定长数据接收）
  UART_DMA_Receive_IT_Init();
  
  // 发送欢迎信息
  char welcome_msg[] = "\r\n=== STM32F103 UART Demo ===\r\n";
  char menu_msg[] = "DMA + IDLE Interrupt Mode Ready!\r\n";
  char prompt_msg[] = "Echo mode enabled. Type something...\r\n";
  
  // 等待串口就绪
  HAL_Delay(100);
  
  // 发送欢迎信息
  UART_SendData((uint8_t*)welcome_msg, strlen(welcome_msg));
  HAL_Delay(50);
  UART_SendData((uint8_t*)menu_msg, strlen(menu_msg));
  HAL_Delay(50);
  UART_SendData((uint8_t*)prompt_msg, strlen(prompt_msg));
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 检查是否有新数据接收
    if (UART_IsDataReceived())
    {
      // 获取接收到的数据
      uint16_t len = UART_GetReceivedData(rx_buffer, sizeof(rx_buffer));
      
      if (len > 0)
      {
        // Echo 模式：将接收到的数据原样返回
        // 添加换行符使输出更清晰
        memcpy(tx_buffer, rx_buffer, len);
        // 准备详细信息提示
        char info_msg[64];
        snprintf(info_msg, sizeof(info_msg), "\r\n[Received] Length: %d bytes\r\n", len);
        UART_SendData((uint8_t*)info_msg, strlen(info_msg));
        
        // 发送数据内容（十六进制显示）
        UART_SendData((uint8_t*)"Hex: ", 5);
        for (uint16_t i = 0; i < len && i < 16; i++)  // 最多显示16字节
        {
          char hex_buf[4];
          snprintf(hex_buf, sizeof(hex_buf), "%02X ", rx_buffer[i]);
          UART_SendData((uint8_t*)hex_buf, strlen(hex_buf));
        }
        if (len > 16)
        {
          UART_SendData((uint8_t*)"...", 3);
        }
        UART_SendData((uint8_t*)"\r\n", 2);
        // 发送回显数据
        UART_SendData(tx_buffer, len);
        
        // 可选：发送换行符
        // uint8_t newline[] = "\r\n";
        // UART_SendData(newline, 2);
      }
    }
    
    // 其他主循环任务...
    HAL_Delay(1);  // 小延时，避免CPU占用过高
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

#ifdef  USE_FULL_ASSERT
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
