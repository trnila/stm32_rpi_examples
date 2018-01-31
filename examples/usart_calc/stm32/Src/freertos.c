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
#include "usart.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId heartbeatTaskHandle;
osMessageQId problemsHandle;

/* USER CODE BEGIN Variables */
#define PROBLEM_MAX_SIZE 20
#define TXBUF_SIZE 100

extern int pxHigherPriorityTaskWoken;

typedef uint8_t Problem[PROBLEM_MAX_SIZE];

uint8_t rxBuffer[PROBLEM_MAX_SIZE];
int pos = 0;

uint8_t txBuffer[TXBUF_SIZE];
int txHead = 0;
int txTail = 0;
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void heartbeat(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void sendNext();
void sendResponse(const char *msg, int size);
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
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of heartbeatTask */
  osThreadDef(heartbeatTask, heartbeat, osPriorityIdle, 0, 128);
  heartbeatTaskHandle = osThreadCreate(osThread(heartbeatTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of problems */
  osMessageQDef(problems, 64, Problem);
  problemsHandle = osMessageCreate(osMessageQ(problems), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
	configASSERT(HAL_UART_Receive_IT(&huart1, rxBuffer, 1) == HAL_OK);
	Problem problem;
	int a, b;
	char op;
	char resultBuf[12];
	for(;;) {
		configASSERT(xQueueReceive(problemsHandle, &problem, portMAX_DELAY) == pdTRUE);

		int val = sscanf(problem, "%d %c %d", &a, &op, &b);
		if(val != 3) {
			continue;
		}

		//osDelay(100);

		int result = a + b;
		snprintf(resultBuf, sizeof(resultBuf), "%d\n", result);
		sendResponse(resultBuf, strlen(resultBuf));
	}
  /* USER CODE END StartDefaultTask */
}

/* heartbeat function */
void heartbeat(void const * argument)
{
  /* USER CODE BEGIN heartbeat */
	for(;;) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
		osDelay(500);
	}
  /* USER CODE END heartbeat */
}

/* USER CODE BEGIN Application */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(rxBuffer[pos] == '\n') {
		configASSERT(xQueueSendFromISR(problemsHandle, rxBuffer, &pxHigherPriorityTaskWoken) == pdTRUE);
		pos = 0;
	} else {
		pos++;
	}

	int x = HAL_UART_Receive_IT(&huart1, rxBuffer + pos, 1);
	configASSERT(x == HAL_OK);

	if (pos >= sizeof(rxBuffer)) {
		pos = 0;
	}
}

void sendNext() {
	configASSERT(HAL_UART_Transmit_IT(&huart1, txBuffer + txHead, 1) == HAL_OK);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	txHead = (txHead + 1) % TXBUF_SIZE;

	if(txHead != txTail) {
		sendNext();
	}
}

void sendResponse(const char *msg, int size) {
	portENTER_CRITICAL();
	int notify = txHead == txTail;

	for(int i = 0; i < size; i++) {
		txBuffer[txTail] = msg[i];
		txTail = (txTail + 1) % TXBUF_SIZE;
	}

	if(notify) {
		sendNext();
	}

	portEXIT_CRITICAL();
}
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
