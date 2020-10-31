#ifndef LED_MODE_HANDLERS_H
#define LED_MODE_HANDLERS_H

#include <stdint.h>
#include "nrf_sdh_ble.h"

void led_mode_write_handler(uint16_t conn_handle, ble_rileylink_service_t * p_rileylink_service, uint8_t led_state);

#endif // LED_MODE_HANDLERS
