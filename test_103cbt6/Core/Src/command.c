/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    command.c
  * @brief   Command processing functions for state machine control
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
#include "usart.h"
#include <stdio.h>
#include <string.h>

/* USER CODE BEGIN 0 */

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

// 状态定义
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

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/**
  * @brief  Initialize state machine
  * @retval None
  */
void state_machine_init(void)
{
    // 赋值给全局指针
    g_state_init = &init_state;
    g_state_idle = &idle_state;
    g_state_working = &working_state;
    g_state_error = &error_state;
    
    // 初始化状态机上下文
    smf_set_initial(&g_app_smf_ctx.ctx, g_state_init);
    
    // 发送欢迎信息
    UART_SendData((uint8_t*)"State machine demo program\r\n", strlen("State machine demo program\r\n"));
    UART_SendData((uint8_t*)"Type 'help' to see command list\r\n", strlen("Type 'help' to see command list\r\n"));
}

/**
  * @brief  Process commands for state machine control
  * @param  cmd: Command buffer
  * @param  length: Command length
  * @retval None
  */
void process_command(uint8_t *cmd, uint16_t length)
{
    // 确保命令以换行符结束
    if (length < UART_RX_BUFFER_SIZE) cmd[length] = '\0';
    
    // 处理状态切换命令
    if (strcmp((char*)cmd, "init") == 0)
    {
        UART_SendData((uint8_t*)"Switch to INIT state\r\n", strlen("Switch to INIT state\r\n"));
        smf_set_state(&g_app_smf_ctx.ctx, g_state_init);
    }
    else if (strcmp((char*)cmd, "idle") == 0)
    {
        UART_SendData((uint8_t*)"Switch to IDLE state\r\n", strlen("Switch to IDLE state\r\n"));
        smf_set_state(&g_app_smf_ctx.ctx, g_state_idle);
    }
    else if (strcmp((char*)cmd, "working") == 0)
    {
        UART_SendData((uint8_t*)"Switch to WORKING state\r\n", strlen("Switch to WORKING state\r\n"));
        smf_set_state(&g_app_smf_ctx.ctx, g_state_working);
    }
    else if (strcmp((char*)cmd, "error") == 0)
    {
        UART_SendData((uint8_t*)"Switch to ERROR state\r\n", strlen("Switch to ERROR state\r\n"));
        smf_set_state(&g_app_smf_ctx.ctx, g_state_error);
    }
    else if (strcmp((char*)cmd, "status") == 0)
    {
        UART_SendData((uint8_t*)"Current state: ", strlen("Current state: "));
        if (g_app_smf_ctx.ctx.current == g_state_init)
            UART_SendData((uint8_t*)"INIT\r\n", strlen("INIT\r\n"));
        else if (g_app_smf_ctx.ctx.current == g_state_idle)
            UART_SendData((uint8_t*)"IDLE\r\n", strlen("IDLE\r\n"));
        else if (g_app_smf_ctx.ctx.current == g_state_working)
            UART_SendData((uint8_t*)"WORKING\r\n", strlen("WORKING\r\n"));
        else if (g_app_smf_ctx.ctx.current == g_state_error)
            UART_SendData((uint8_t*)"ERROR\r\n", strlen("ERROR\r\n"));
    }
    else if (strcmp((char*)cmd, "help") == 0)
    {
        UART_SendData((uint8_t*)"Command list:\r\n", strlen("Command list:\r\n"));
        UART_SendData((uint8_t*)"init - Switch to INIT state\r\n", strlen("init - Switch to INIT state\r\n"));
        UART_SendData((uint8_t*)"idle - Switch to IDLE state\r\n", strlen("idle - Switch to IDLE state\r\n"));
        UART_SendData((uint8_t*)"working - Switch to WORKING state\r\n", strlen("working - Switch to WORKING state\r\n"));
        UART_SendData((uint8_t*)"error - Switch to ERROR state\r\n", strlen("error - Switch to ERROR state\r\n"));
        UART_SendData((uint8_t*)"status - Show current state\r\n", strlen("status - Show current state\r\n"));
        UART_SendData((uint8_t*)"help - Show help information\r\n", strlen("help - Show help information\r\n"));
    }
    else
    {
        UART_SendData((uint8_t*)"Unknown command, type 'help' to see command list\r\n", strlen("Unknown command, type 'help' to see command list\r\n"));
    }
}

/* USER CODE END 1 */