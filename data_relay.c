
#include "nrf_log.h"

#include "rileylink_service.h"
#include "led_mode_handlers.h"

void data_relay_ble_write_handler(uint16_t conn_handle, ble_rileylink_service_t * p_rileylink_service, const uint8_t *data, uint16_t data_len)
{
    NRF_LOG_INFO("Data written: %d bytes.", data_len);
}
