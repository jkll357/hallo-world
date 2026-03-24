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
#include "smf.h"
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

/* Private variables ---------------------------------------------------------

USER CODE BEGIN PV */
// 接收缓冲区
static uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
static uint16_t rx_index = 0;

// 命令处理函数
void process_command(uint8_t *cmd, uint16_t length)
{
    // 确保命令以换行符结束
    cmd[length] = '\0';
    
    // 处理状态切换命令
    if (strcmp((char*)cmd, "init") == 0)
    {
        UART_SendData((uint8_t*)"切换到初始化状态\r\n", strlen("切换到初始化状态\r\n"));
        smf_set_state(&g_app_smf_ctx.ctx, g_state_init);
    }
    else if (strcmp((char*)cmd, "idle") == 0)
    {
        UART_SendData((uint8_t*)"切换到空闲状态\r\n", strlen("切换到空闲状态\r\n"));
        smf_set_state(&g_app_smf_ctx.ctx, g_state_idle);
    }
    else if (strcmp((char*)cmd, "working") == 0)
    {
        UART_SendData((uint8_t*)"切换到工作状态\r\n", strlen("切换到工作状态\r\n"));
        smf_set_state(&g_app_smf_ctx.ctx, g_state_working);
    }
    else if (strcmp((char*)cmd, "error") == 0)
    {
        UART_SendData((uint8_t*)"切换到错误状态\r\n", strlen("切换到错误状态\r\n"));
        smf_set_state(&g_app_smf_ctx.ctx, g_state_error);
    }
    else if (strcmp((char*)cmd, "status") == 0)
    {
        UART_SendData((uint8_t*)"当前状态: ", strlen("当前状态: "));
        if (g_app_smf_ctx.ctx.current == g_state_init)
            UART_SendData((uint8_t*)"初始化\r\n", strlen("初始化\r\n"));
        else if (g_app_smf_ctx.ctx.current == g_state_idle)
            UART_SendData((uint8_t*)"空闲\r\n", strlen("空闲\r\n"));
        else if (g_app_smf_ctx.ctx.current == g_state_working)
            UART_SendData((uint8_t*)"工作\r\n", strlen("工作\r\n"));
        else if (g_app_smf_ctx.ctx.current == g_state_error)
            UART_SendData((uint8_t*)"错误\r\n", strlen("错误\r\n"));
    }
    else if (strcmp((char*)cmd, "help") == 0)
    {
        UART_SendData((uint8_t*)"命令列表:\r\n", strlen("命令列表:\r\n"));
        UART_SendData((uint8_t*)"init - 切换到初始化状态\r\n", strlen("init - 切换到初始化状态\r\n"));
        UART_SendData((uint8_t*)"idle - 切换到空闲状态\r\n", strlen("idle - 切换到空闲状态\r\n"));
        UART_SendData((uint8_t*)"working - 切换到工作状态\r\n", strlen("working - 切换到工作状态\r\n"));
        UART_SendData((uint8_t*)"error - 切换到错误状态\r\n", strlen("error - 切换到错误状态\r\n"));
        UART_SendData((uint8_t*)"status - 显示当前状态\r\n", strlen("status - 显示当前状态\r\n"));
        UART_SendData((uint8_t*)"help - 显示帮助信息\r\n", strlen("help - 显示帮助信息\r\n"));
    }
    else
    {
        UART_SendData((uint8_t*)"未知命令，请输入 'help' 查看命令列表\r\n", strlen("未知命令，请输入 'help' 查看命令列表\r\n"));
    }
}

// 串口接收回调函数
void UART_RxCallback(uint8_t *data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        if (data[i] == '\r' || data[i] == '\n')
        {
            // 命令结束，处理命令
            if (rx_index > 0)
            {
                process_command(rx_buffer, rx_index);
                rx_index = 0;
            }
        }
        else if (rx_index < UART_RX_BUFFER_SIZE - 1)
        {
            // 存储接收到的字符
            rx_buffer[rx_index++] = data[i];
        }
    }
}

// 状态机变量
struct app_smf_ctx g_app_smf_ctx;

// 状态定义
struct smf_state *g_state_init;
struct smf_state *g_state_idle;
struct smf_state *g_state_working;
struct smf_state *g_state_error;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------
USER CODE BEGIN 0 */

// 状态处理函数

// 初始化状态
static void state_init_entry(void *obj)
{
    struct app_smf_ctx *app_ctx = (struct app_smf_ctx *)obj;
    char msg[64];
    snprintf(msg, sizeof(msg), "[State Machine] Entering INIT state\r\n");
    UART_SendData((uint8_t*)msg, strlen(msg));
    app_ctx->state_counter = 0;
    app_ctx->error_count = 0;
}

static enum smf_state_result state_init_run(void *obj)
{
    struct app_smf_ctx *app_ctx = (struct app_smf_ctx *)obj;
    // 初始化完成后跳转到空闲状态
    smf_set_state(&app_ctx->ctx, g_state_idle);
    return SMF_EVENT_HANDLED;
}

// 空闲状态
static void state_idle_entry(void *obj)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "[State Machine] Entering IDLE state\r\n");
    UART_SendData((uint8_t*)msg, strlen(msg));
}

static enum smf_state_result state_idle_run(void *obj)
{
    struct app_smf_ctx *app_ctx = (struct app_smf_ctx *)obj;
    app_ctx->state_counter++;
    
    // 每1000次循环跳转到工作状态
    if (app_ctx->state_counter >= 1000)
    {
        smf_set_state(&app_ctx->ctx, g_state_working);
        app_ctx->state_counter = 0;
    }
    
    // 模拟错误检测
    if (app_ctx->error_count > 5)
    {
        smf_set_state(&app_ctx->ctx, g_state_error);
    }
    
    return SMF_EVENT_HANDLED;
}

// 工作状态
static void state_working_entry(void *obj)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "[State Machine] Entering WORKING state\r\n");
    UART_SendData((uint8_t*)msg, strlen(msg));
}

static enum smf_state_result state_working_run(void *obj)
{
    struct app_smf_ctx *app_ctx = (struct app_smf_ctx *)obj;
    app_ctx->state_counter++;
    
    // 工作500次循环后返回空闲状态
    if (app_ctx->state_counter >= 500)
    {
        smf_set_state(&app_ctx->ctx, g_state_idle);
        app_ctx->state_counter = 0;
    }
    
    return SMF_EVENT_HANDLED;
}

// 错误状态
static void state_error_entry(void *obj)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "[State Machine] Entering ERROR state\r\n");
    UART_SendData((uint8_t*)msg, strlen(msg));
}

static enum smf_state_result state_error_run(void *obj)
{
    struct app_smf_ctx *app_ctx = (struct app_smf_ctx *)obj;
    app_ctx->state_counter++;
    
    // 错误状态持续1000次循环后返回空闲状态
    if (app_ctx->state_counter >= 1000)
    {
        smf_set_state(&app_ctx->ctx, g_state_idle);
        app_ctx->state_counter = 0;
        app_ctx->error_count = 0; // 重置错误计数
    }
    
    return SMF_EVENT_HANDLED;
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
  
  // 初始化状态机
  // 定义状态
  static struct smf_state init_state = {
    .entry = state_init_entry,
    .run = state_init_run,
    .exit = NULL
  };
  
  static struct smf_state idle_state = {
    .entry = state_idle_entry,
    .run = state_idle_run,
    .exit = NULL
  };
  
  static struct smf_state working_state = {
    .entry = state_working_entry,
    .run = state_working_run,
    .exit = NULL
  };
  
  static struct smf_state error_state = {
    .entry = state_error_entry,
    .run = state_error_run,
    .exit = NULL
  };
  
  // 赋值给全局指针
  g_state_init = &init_state;
  g_state_idle = &idle_state;
  g_state_working = &working_state;
  g_state_error = &error_state;
  
  // 初始化状态机上下文
  smf_set_initial(&g_app_smf_ctx.ctx, g_state_init);
  
  // 发送欢迎信息
  UART_SendData((uint8_t*)"状态机演示程序\r\n", strlen("状态机演示程序\r\n"));
  UART_SendData((uint8_t*)"输入 'help' 查看命令列表\r\n", strlen("输入 'help' 查看命令列表\r\n"));
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 运行状态机
    smf_run_state(&g_app_smf_ctx.ctx);
    
    // 短暂延时，避免占用过多CPU资源
    HAL_Delay(1);
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
