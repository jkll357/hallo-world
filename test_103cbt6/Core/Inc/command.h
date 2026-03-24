/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    command.h
  * @brief   Header file for command.c
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
#ifndef __COMMAND_H__
#define __COMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Prototypes */

/**
  * @brief  Initialize state machine
  * @retval None
  */
void state_machine_init(void);

/**
  * @brief  Process commands for state machine control
  * @param  cmd: Command buffer
  * @param  length: Command length
  * @retval None
  */
void process_command(uint8_t *cmd, uint16_t length);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_H__ */