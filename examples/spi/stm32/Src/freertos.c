/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "stm32f1xx.h"
#include "spi.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId spiHandle;
osMessageQId commandsHandle;

/* USER CODE BEGIN Variables */
extern uint8_t xSwitchRequired;

#define MAX_ARGS 4

typedef enum {
	CMD_READ = 0x10,
	CMD_SET,
	CMD_ADD,
	CMD_SUB,
	CMD_AND,
	CMD_OR,
} CommandType;

typedef union {
	uint8_t bytes[sizeof(int)];
	int word;
} Number;

typedef struct {
	uint8_t cmd;
	uint8_t argLen;
	Number args[MAX_ARGS];
} Command;

typedef enum {
	STATE_START,
	STATE_ARGLEN,
	STATE_READ,
	STATE_WRITE
} State;

Command current;
State state;
Number number;
uint8_t nil[MAX_ARGS * sizeof(int)];
uint8_t queueSize;
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void spiTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

void start_over();

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of spi */
  osThreadDef(spi, spiTask, osPriorityIdle, 0, 128);
  spiHandle = osThreadCreate(osThread(spi), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of commands */
  osMessageQDef(commands, 16, Command);
  commandsHandle = osMessageCreate(osMessageQ(commands), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(500);
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
  }
  /* USER CODE END StartDefaultTask */
}

/* spiTask function */
void spiTask(void const * argument)
{
  /* USER CODE BEGIN spiTask */
	start_over();
	Command cmd;

	for(;;) {
		configASSERT(xQueuePeek(commandsHandle, &cmd, portMAX_DELAY) == pdTRUE);

		HAL_Delay(2000);

		for(int i = 0; i < cmd.argLen; i++) {
			switch (cmd.cmd) {
				case CMD_SET:
					number.word = cmd.args[0].word;
					break;

				case CMD_ADD:
					number.word += cmd.args[i].word;
					break;

				case CMD_SUB:
					number.word -= cmd.args[i].word;
					break;

				case CMD_AND:
					number.word &= cmd.args[i].word;
					break;

				case CMD_OR:
					number.word |= cmd.args[i].word;
					break;
			}
		}

		configASSERT(xQueueReceive(commandsHandle, &cmd, portMAX_DELAY) == pdTRUE);
	}
  /* USER CODE END spiTask */
}

/* USER CODE BEGIN Application */
void start_over() {
	state = STATE_START;
	queueSize = uxQueueMessagesWaitingFromISR(commandsHandle);
	configASSERT(HAL_SPI_TransmitReceive_IT(&hspi1, &queueSize, &current.cmd, 1) == HAL_OK);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
	start_over();
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	if(state == STATE_START) {
		if(current.cmd == CMD_READ) {
			state = STATE_WRITE;
			configASSERT(HAL_SPI_TransmitReceive_IT(&hspi1, number.bytes, nil, sizeof(number)) == HAL_OK);
		} else {
			state = STATE_ARGLEN;
			configASSERT(HAL_SPI_TransmitReceive_IT(&hspi1, nil, &current.argLen, sizeof(current.argLen)) == HAL_OK);
		}
	} else if(state == STATE_READ) {
		BaseType_t ret = xQueueSendFromISR(commandsHandle, &current, &xSwitchRequired);
		configASSERT(ret == pdTRUE || ret == errQUEUE_FULL);
		start_over();
	} else if(state == STATE_ARGLEN) {
		state = STATE_READ;
		if(current.argLen > 0 && current.argLen <= MAX_ARGS) {
			configASSERT(HAL_SPI_TransmitReceive_IT(&hspi1, nil, current.args, sizeof(current.args[0]) * current.argLen) == HAL_OK);
		} else {
			start_over();
		}
	} else {
		start_over();
	}
}
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
