#include "erpc_client_setup.h"

erpc_transport_t transport;

void erpc_setup(const char* serial_path, const int baud_rate) {
	transport = erpc_transport_serial_init(serial_path, baud_rate);
	erpc_mbf_t message_buffer_factory = erpc_mbf_dynamic_init();
	erpc_client_init(transport, message_buffer_factory);
}