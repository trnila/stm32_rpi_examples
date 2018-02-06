#include "uart_hal_transport.h"

#include <cstdio>
#include <cassert>
#include "cmsis_os.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

const int SIZE = 64;
const int TIMEOUT = 50;
static uint8_t buffer[SIZE];
static volatile uint32_t head;
static volatile uint32_t tail;
static UART_HandleTypeDef *huart;
osThreadId thread;
uint8_t *target;
int size;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

UartHalTransport::UartHalTransport(UART_HandleTypeDef *uartDrv)
: m_uartDrv(uartDrv)
{
	huart = uartDrv;
	configASSERT(HAL_UART_Receive_DMA(huart, buffer, SIZE) == HAL_OK);
}

UartHalTransport::~UartHalTransport()
{
}

/* Transfer callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *hspi) {
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
	for(int i = 0; i < size; i++) {
		target[i] = buffer[i];
	}

	osThreadResume(thread);
}

erpc_status_t UartHalTransport::init() {
	thread = osThreadGetId();
	return kErpcStatus_Success;
}

erpc_status_t UartHalTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
	target = data;
	::size = size;
	if(size > 0) {
		osThreadSuspend(NULL);
	}
    return kErpcStatus_Success;
}

erpc_status_t UartHalTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    HAL_UART_Transmit(m_uartDrv, (uint8_t*) data, size, HAL_MAX_DELAY);

    return kErpcStatus_Success;
}
