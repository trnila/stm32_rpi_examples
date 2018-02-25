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
#include "gpio_banks.h"
#include "main.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId bank0Handle;
osThreadId bank1Handle;
osThreadId sendToRpiHandle;

/* USER CODE BEGIN Variables */
const int pin_mask = (1 << GPIO_BANKS_WIDE) - 1;

const GPIO_Bank banks[][GPIO_BANKS_WIDE] = {
	{
		{GPIOA, GPIO_PIN_0},
		{GPIOA, GPIO_PIN_1},
		{GPIOA, GPIO_PIN_2},
		{GPIOA, GPIO_PIN_3},
		{GPIOA, GPIO_PIN_4},
		{GPIOA, GPIO_PIN_5},
		{GPIOA, GPIO_PIN_6},
		{GPIOA, GPIO_PIN_7},
	},
	{
		{GPIOB, GPIO_PIN_5},
		{GPIOB, GPIO_PIN_4},
		{GPIOB, GPIO_PIN_3},
		{GPIOA, GPIO_PIN_15},
		{GPIOA, GPIO_PIN_12},
		{GPIOA, GPIO_PIN_11},
		{GPIOA, GPIO_PIN_8},
		{GPIOB, GPIO_PIN_15}
	},
};

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void bank0Task(void const * argument);
void bank1Task(void const * argument);
void sendToRpiTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

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
  /* definition and creation of bank0 */
  osThreadDef(bank0, bank0Task, osPriorityNormal, 0, 128);
  bank0Handle = osThreadCreate(osThread(bank0), NULL);

  /* definition and creation of bank1 */
  osThreadDef(bank1, bank1Task, osPriorityIdle, 0, 128);
  bank1Handle = osThreadCreate(osThread(bank1), NULL);

  /* definition and creation of sendToRpi */
  osThreadDef(sendToRpi, sendToRpiTask, osPriorityIdle, 0, 128);
  sendToRpiHandle = osThreadCreate(osThread(sendToRpi), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* bank0Task function */
void bank0Task(void const * argument)
{

  /* USER CODE BEGIN bank0Task */
	// set all pins in bank 0 to output
	gpio_bank_setup(&banks[0], ~0u);
	int value = 0;
	for(;;) {
		gpio_bank_set(&banks[0], value);

		value = (value + 1) & pin_mask;
		osDelay(500);
	}
  /* USER CODE END bank0Task */
}

/* bank1Task function */
void bank1Task(void const * argument)
{
  /* USER CODE BEGIN bank1Task */
	// set all pins in bank 1 to output
	gpio_bank_setup(&banks[1], ~0u);
	int value = pin_mask;
	for(;;) {
		gpio_bank_set(&banks[1], value);

		value = (value - 1) & pin_mask;
		osDelay(100);
	}
  /* USER CODE END bank1Task */
}

/* sendToRpiTask function */
void sendToRpiTask(void const * argument)
{
  /* USER CODE BEGIN sendToRpiTask */
  for(;;)  {
	  osDelay(25);
	  HAL_GPIO_TogglePin(toRpi_GPIO_Port, toRpi_Pin);
  }
  /* USER CODE END sendToRpiTask */
}

/* USER CODE BEGIN Application */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if(GPIO_Pin == fromRpi_Pin) {
		HAL_GPIO_TogglePin(toRpi_GPIO_Port, toRpi_Pin);
	} else {
		// unknown gpio interrupt
		asm("bkpt #0");
	}
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
