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
uint8_t rx[2];
uint8_t tx[2];
int dir;
int pos = 0;

#define DIR_READ 0
#define DIR_WRITE 1


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
	configASSERT(HAL_I2C_EnableListen_IT(&hi2c1) == HAL_OK);
}

char ret = 0;
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	// transmit done
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	pos++;

	//configASSERT(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, rx, 1, I2C_LAST_FRAME) == HAL_OK);

	/*

	pos++;
	if(pos == 2) {
		return;
	}

	configASSERT(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, rx + pos, 1, I2C_NEXT_FRAME) == HAL_OK);
	 */
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
	pos = 0;

	if(TransferDirection == I2C_DIRECTION_TRANSMIT) {
		configASSERT(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, rx, 1, I2C_FIRST_FRAME) == HAL_OK);
	} else if(TransferDirection == I2C_DIRECTION_RECEIVE) {
		tx[0]++;
		int val = HAL_I2C_Slave_Sequential_Transmit_IT(hi2c, tx, 1, I2C_FIRST_AND_LAST_FRAME);
		configASSERT(val == HAL_OK);
	} else {
		asm("bkpt #1");
	}
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
	configASSERT(HAL_I2C_EnableListen_IT(&hi2c1) == HAL_OK);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
	asm("bkpt #1");
	if(hi2c->ErrorCode != HAL_I2C_ERROR_AF) {
		//start_over();
	} else {
		asm("bkpt #1");
	}
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) {
	asm("bkpt #1");
}
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/