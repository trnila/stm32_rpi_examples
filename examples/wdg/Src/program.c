#include <iwdg.h>
#include "stm32f1xx_hal.h"

void program() {
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

	// reset watchdog counter
	HAL_IWDG_Refresh(&hiwdg);

	int i = 3;
	for(;;) {
		i--;
		HAL_Delay(400);

		if(i <= 0) {
			// hang
			for(;;);
		}

		// reset watchdog counter
		HAL_IWDG_Refresh(&hiwdg);
	}
}