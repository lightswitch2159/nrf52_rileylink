#ifndef DATA_RELAY_H
#define DATA_RELAY_H

#include <stdint.h>
#include "nrf_sdh_ble.h"

void data_relay_init(ble_rileylink_service_t * p_rileylink_service);
void data_relay_ble_write_handler(uint16_t conn_handle, ble_rileylink_service_t * p_rileylink_service, const uint8_t *data, uint16_t data_len);
void data_relay_spi_response_handler(const uint8_t *data, uint8_t length);

#endif // DATA_RELAY_H

