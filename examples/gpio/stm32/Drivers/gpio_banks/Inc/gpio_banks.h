#pragma once

#include "stm32f1xx.h"

#ifndef GPIO_BANKS_WIDE
#define GPIO_BANKS_WIDE 8
#endif

typedef struct {
	GPIO_TypeDef *port;
	int pin;
} GPIO_Bank;

void gpio_bank_setup(const GPIO_Bank *bank, unsigned int state);
void gpio_bank_set(const GPIO_Bank *bank, unsigned int state);