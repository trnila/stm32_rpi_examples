#include "messages.h"
#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "pins.h"

TaskHandle_t task;
EventGroupHandle_t eventGroup;
uint8_t data[2];
uint32_t delay = 500;
bool running = true;
int led = 0;
int res = 0;

uint8_t set_led(uint8_t bank, uint8_t state) {
	gpio_set(bank, state);
	return (bank << 8) + state;
}

bool led_strip(uint32_t delay_, Direction dir) {
	delay = delay_;

	xTaskNotifyGive(task);
	return true;
}

bool led_ctrl(Ctrl ctrl) {
	if(ctrl == Ctrl_pause) {
		running = false;
	} else {
		running = true;

		if(ctrl == Ctrl_restart) {
			led = 0;
		}
	}

	xTaskNotifyGive(task);
	return true;
}

void handleCtrl() {
	task = xTaskGetCurrentTaskHandle();
	configASSERT(task != NULL);

	gpio_setup(0, 0x00);

	for(;;) {
		ulTaskNotifyTake(pdTRUE, running ? delay : portMAX_DELAY);

		gpio_set(0, running ? 1 << led : 0x00);

		led++;
		led &= 7;
	}
}

int32_t add(int32_t a, int32_t b) {
	return a + b;
}