#include "stm32f1xx.h"

#define WIDE 8

typedef struct {
	GPIO_TypeDef *port;
	int pin;
} Mapping;

const Mapping mappings[][WIDE] = {
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
	{
		{GPIOA, GPIO_PIN_0},
		{GPIOA, GPIO_PIN_1},
		{GPIOA, GPIO_PIN_2},
		{GPIOA, GPIO_PIN_3},
		{GPIOA, GPIO_PIN_4},
		{GPIOA, GPIO_PIN_5},
		{GPIOA, GPIO_PIN_6},
		{GPIOA, GPIO_PIN_7},
	}

};

// 0 means output
// 1 means input
void gpio_setup(uint8_t bank, uint8_t state) {
	assert_param(bank == 0 || bank == 1);
	const Mapping *mapping = mappings[bank];

	for(int i = 0; i < WIDE; i++) {
		GPIO_InitTypeDef GPIO_InitStruct;

		GPIO_InitStruct.Pin = mapping[i].pin;
		GPIO_InitStruct.Mode = state & (1 << i) ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(mapping[i].port, &GPIO_InitStruct);
	}
}

void gpio_set(uint8_t bank, uint8_t state) {
	assert_param(bank == 0 || bank == 1);
	const Mapping *mapping = mappings[bank];

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_All, GPIO_PIN_SET);

	for(int i = 0; i < WIDE; i++) {
		GPIO_PinState pinState;
		if(state & (1 << i)) {
			pinState = GPIO_PIN_SET;
		} else {
			pinState = GPIO_PIN_RESET;
		}

		HAL_GPIO_WritePin(mapping[i].port, mapping[i].pin, pinState);
	}
}