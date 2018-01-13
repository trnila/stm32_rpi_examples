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
#include "adc.h"
#include "usart.h"
#include "spi.h"
#include <string.h>
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId touchHandle;

/* USER CODE BEGIN Variables */
#define YP GPIO_PIN_0
#define XM GPIO_PIN_1
#define YM GPIO_PIN_2
#define XP GPIO_PIN_3

#define SAMPLES_NUM 50

#define AXIS_Y 0
#define AXIS_X 1

#define ADS_MEASURE_Y 0b001
#define ADS_MEASURE_X 0b101
#define ADS_MEASURE_Z1 0b011
#define ADS_MEASURE_Z2 0b100


typedef enum {
	STATE_START,
	STATE_READ,
} State;

int samples[SAMPLES_NUM];

int curr[2];

int min[2];
int max[2];

char printBuffer[50];

State state;
uint8_t rx[2];
uint8_t tx[2];

int send_irq = 1;



/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void touchTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void start_over();

int calcPercent(int current, int axis) {
	int percent = (((float) current - min[axis]) / (max[axis] - min[axis]) * 100.0);
	if(percent < 0) {
		return 0;
	} else if(percent > 100) {
		return 100;
	}

	return percent;
}

void pinMode(int pin, int mode) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void digitalWrite(int pin, int state) {
	HAL_GPIO_WritePin(GPIOA, pin, state);
}

int adcRead() {
	configASSERT(HAL_ADC_Start(&hadc1) == HAL_OK);
	configASSERT(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK);

	int val = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return val;
}

int measure(int axis) {
	for(int i = 0; i < SAMPLES_NUM; i++) {
		samples[i] = adcRead();
	}
	for(int i = 0; i < SAMPLES_NUM; i++) {
		for(int j = 0; j < SAMPLES_NUM - 1; j++) {
			if(samples[j] > samples[j + 1]) {
				int tmp = samples[j];
				samples[j] = samples[j + 1];
				samples[j + 1] = tmp;
			}
		}
	}

	int current = samples[SAMPLES_NUM / 2];

	curr[axis] = current;

	return current;
}


int is_touched() {
	return !(curr[AXIS_Y] > 2000 && curr[AXIS_Y] < 2180 &&
			curr[AXIS_X] > 2000 && curr[AXIS_X] < 2170);

}

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

  /* definition and creation of touch */
  osThreadDef(touch, touchTask, osPriorityIdle, 0, 128);
  touchHandle = osThreadCreate(osThread(touch), NULL);

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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
  }
  /* USER CODE END StartDefaultTask */
}

/* touchTask function */
void touchTask(void const * argument)
{
  /* USER CODE BEGIN touchTask */
	start_over();

	min[AXIS_Y] = 506;
	max[AXIS_Y] = 3500;

	min[AXIS_X] = 211;
	max[AXIS_X] = 3500;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

	int X, Y;
	// precent
	int was_touched = 0;
	for(;;) {
		ADC_ChannelConfTypeDef sConfig;

		// measure Y
		pinMode(YP, GPIO_MODE_ANALOG);
		pinMode(YM, GPIO_MODE_ANALOG);
		pinMode(XP, GPIO_MODE_OUTPUT_PP);
		pinMode(XM, GPIO_MODE_OUTPUT_PP);

		sConfig.Channel = ADC_CHANNEL_0;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
		configASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);

		digitalWrite(XP, GPIO_PIN_SET);
		digitalWrite(XM, GPIO_PIN_RESET);

		Y = measure(AXIS_Y);

		// ================== X AXIS ================
		pinMode(XP, GPIO_MODE_ANALOG);
		pinMode(XM, GPIO_MODE_ANALOG);
		pinMode(YP, GPIO_MODE_OUTPUT_PP);
		pinMode(YM, GPIO_MODE_OUTPUT_PP);

		sConfig.Channel = ADC_CHANNEL_1;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
		configASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);

		digitalWrite(YP, GPIO_PIN_SET);
		digitalWrite(YM, GPIO_PIN_RESET);

		X = measure(AXIS_X);

		snprintf(printBuffer, sizeof(printBuffer), "X=%d Y=%d XP=%d XY=%d\n\r", X, Y, calcPercent(X, AXIS_X), calcPercent(Y, AXIS_Y));
		HAL_UART_Transmit(&huart1, printBuffer, strlen(printBuffer), HAL_MAX_DELAY);

		if(send_irq) {
			if (is_touched()) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
			} else  {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
			}
		}
	}
  /* USER CODE END touchTask */
}

/* USER CODE BEGIN Application */
void start_over() {
	state = STATE_START;
	tx[0] = 0;
	configASSERT(HAL_SPI_TransmitReceive_IT(&hspi1, tx, rx, 1) == HAL_OK);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
	start_over();
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	if(state == STATE_START) {
		// start bit missing
		if(!(rx[0] & (1 << 7))) {
			start_over();
		} else {

			uint8_t mode = (rx[0] & 0x70) >> 4;
			send_irq = (rx[0] & 0x3) == 0;

			tx[0] = tx[1] = 0;
			if(mode == 0b001) {
				tx[0] = (curr[AXIS_Y] >> 8) & 0xFFFF;
				tx[1] = curr[AXIS_Y] & 0xF;
			} else if(mode == 0b101) {
				tx[0] = (curr[AXIS_X] >> 8) & 0xFFFF;
				tx[1] = curr[AXIS_X] & 0xF;
			} else if(mode == 0b100 || mode == 0b011) {
				int pressure = 32000;

				tx[0] = (pressure >> 8) & 0xFFFF;
				tx[1] = pressure & 0xF;
			} else {
				// TODO: wat?
			}

			state = STATE_READ;
			configASSERT(HAL_SPI_TransmitReceive_IT(&hspi1, tx, rx, 2) == HAL_OK);
		}
	} else if(state == STATE_READ) {
		start_over();
	} else {
		start_over();
	}
}


     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
