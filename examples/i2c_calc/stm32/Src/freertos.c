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
#include "i2c.h"
#include "stm32f1xx.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;

/* USER CODE BEGIN Variables */
#define BUF_MAX 32

union {
	uint8_t bytes[BUF_MAX];
	struct {
		uint8_t cmd;
		uint32_t nums[(BUF_MAX + sizeof(uint32_t))/ sizeof(uint32_t)];
	} __attribute__((packed));
} rx;

union {
	uint8_t bytes[BUF_MAX];
	uint32_t nums[(BUF_MAX + sizeof(uint32_t))/ sizeof(uint32_t)];
} tx;


int dir;
int pos = 0;

uint8_t ram[255];

#define DIR_READ 0
#define DIR_WRITE 1

#define CMD_READ 0x10
#define CMD_SET 0x11
#define CMD_ADD 0x12
#define CMD_SUB 0x13
#define CMD_AND 0x14
#define CMD_OR 0x15
uint32_t result = 0;


/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);

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

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
	start_over();
	for(;;) {
		osDelay(500);
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */
void start_over() {
	pos = 0;
	for(int i = 0; i < 18; i++) {
		rx.bytes[i] = 0;
		tx.bytes[i] = 0;
	}
	configASSERT(HAL_I2C_EnableListen_IT(&hi2c1) == HAL_OK);
}

void process() {
	switch(rx.cmd) {
		case CMD_SET:
			result = rx.nums[0];
			break;
		case CMD_ADD:
			result += rx.nums[0];
			break;
		case CMD_SUB:
			result -= rx.nums[0];
			break;
		case CMD_AND:
			result &= rx.nums[0];
			break;
		case CMD_OR:
			result |= rx.nums[0];
			break;
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	// process

	// increment
	pos++;
	configASSERT(pos < BUF_MAX);

	configASSERT(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, rx.bytes + pos, 1, I2C_LAST_FRAME) == HAL_OK);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
	dir = TransferDirection;
	pos = 0;
	if(TransferDirection == I2C_DIRECTION_TRANSMIT) {
		configASSERT(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, rx.bytes + pos, 1, I2C_FIRST_FRAME) == HAL_OK);
	} else if(TransferDirection == I2C_DIRECTION_RECEIVE) {
		tx.nums[0] = result;

		hi2c->State = HAL_I2C_STATE_LISTEN;
		configASSERT(HAL_I2C_Slave_Sequential_Transmit_IT(hi2c, tx.bytes, sizeof(tx.nums[0]), I2C_FIRST_AND_LAST_FRAME) == HAL_OK);
	} else {
		asm("bkpt #1");
	}
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
	start_over();
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
	if(dir == I2C_DIRECTION_TRANSMIT) {
		process();
	} else {
		asm("bkpt #1");
	}
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) {
	asm("bkpt #1");
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
