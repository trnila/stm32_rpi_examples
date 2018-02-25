#include <stdio.h>
#include <wiringPi.h>
#include <semaphore.h>

const int PIN = 19;

sem_t sem;

int count = 0;
void irq_received() {
	count++;
	sem_post(&sem);
}

int main() {
	sem_init(&sem, 0, 0);

	wiringPiSetupGpio();
	pinMode(PIN, INPUT);

	wiringPiISR(PIN, INT_EDGE_RISING, irq_received); 

	for(;;) {
		sem_wait(&sem);
		printf("Rising count: %d\n", count);
	}
}
