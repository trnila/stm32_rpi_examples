#include "uart_hal_transport.h"

#include <cstdio>
#include <cassert>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

const int SIZE = 128;
const int TIMEOUT = 5;
static uint8_t buffer[SIZE];
static volatile uint32_t head;
static volatile uint32_t tail;
static UART_HandleTypeDef *huart;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

UartHalTransport::UartHalTransport(UART_HandleTypeDef *uartDrv)
: m_uartDrv(uartDrv)
{
	huart = uartDrv;
	assert(HAL_UART_Receive_IT(huart, buffer, 1) == HAL_OK);
}

UartHalTransport::~UartHalTransport()
{
}

/* Transfer callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *hspi) {
	tail = (tail + 1) % SIZE;
	HAL_UART_Receive_IT(huart, buffer + tail, 1);
}

erpc_status_t UartHalTransport::init()
{
	return kErpcStatus_Success;
}

erpc_status_t UartHalTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
	int start;
    for(uint32_t i = 0; i < size; i++) {
    	while(head == tail) {
    		if(i != 0 && HAL_GetTick() - start > TIMEOUT) {
				return kErpcStatus_Timeout;
			}
    	}

    	start = HAL_GetTick();

    	data[i] = buffer[head];
    	head = (head + 1) % SIZE;
    }

    return kErpcStatus_Success;
}

erpc_status_t UartHalTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    HAL_UART_Transmit(m_uartDrv, (uint8_t*) data, size, HAL_MAX_DELAY);

    return kErpcStatus_Success;
}
