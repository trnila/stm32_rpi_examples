#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "i2c.h"

#define MEM_SIZE 16
uint8_t memory[MEM_SIZE];
uint8_t pointer = 0;

void i2c_init() {
	configASSERT(HAL_I2C_EnableListen_IT(&hi2c1) == HAL_OK);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	configASSERT(pointer < MEM_SIZE);

	configASSERT(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, memory + pointer, 1, I2C_LAST_FRAME) == HAL_OK);
	pointer++;
	if(pointer >= MEM_SIZE) {
		pointer = 0;
	}
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
	if(TransferDirection == I2C_DIRECTION_TRANSMIT) {
		// read pointer address
		configASSERT(HAL_I2C_Slave_Sequential_Receive_IT(hi2c, &pointer, 1, I2C_FIRST_FRAME) == HAL_OK);
	} else if(TransferDirection == I2C_DIRECTION_RECEIVE) {
		hi2c->State = HAL_I2C_STATE_LISTEN;

		uint8_t remaining = MEM_SIZE - pointer;

		// decrement, because it was previously incremented by HAL_I2C_SlaveRxCpltCallback
		if(pointer == 0) {
			pointer = MEM_SIZE - 1;
		} else {
			pointer--;
		}

		configASSERT(pointer < MEM_SIZE);
		configASSERT(HAL_I2C_Slave_Sequential_Transmit_IT(hi2c, memory + pointer, remaining, I2C_FIRST_AND_LAST_FRAME) == HAL_OK);
		pointer = (pointer + remaining) % MEM_SIZE;
	} else {
		asm("bkpt #1");
	}
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_init();
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
	// overflow error occurs when the other end quits transfer
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	uint8_t remaining = MEM_SIZE - pointer;
	configASSERT(HAL_I2C_Slave_Sequential_Transmit_IT(hi2c, memory + pointer, remaining, I2C_FIRST_AND_LAST_FRAME) == HAL_OK);
	pointer = (pointer + remaining) % MEM_SIZE;
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) {
	asm("bkpt #1");
}
