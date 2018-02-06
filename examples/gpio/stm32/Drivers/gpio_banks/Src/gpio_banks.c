#include "gpio_banks.h"

void gpio_bank_setup(const GPIO_Bank *bank, unsigned int state) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	for(int i = 0; i < GPIO_BANKS_WIDE; i++) {
		GPIO_InitTypeDef GPIO_InitStruct;

		GPIO_InitStruct.Pin = bank[i].pin;
		GPIO_InitStruct.Mode = state & (1 << i) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(bank[i].port, &GPIO_InitStruct);
	}
}

void gpio_bank_set(const GPIO_Bank *bank, unsigned int state) {
	for(int i = 0; i < GPIO_BANKS_WIDE; i++) {
		GPIO_PinState pinState;
		if(state & (1 << i)) {
			pinState = GPIO_PIN_SET;
		} else {
			pinState = GPIO_PIN_RESET;
		}

		HAL_GPIO_WritePin(bank[i].port, bank[i].pin, pinState);
	}
}