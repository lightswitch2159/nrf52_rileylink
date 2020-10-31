#include "nrf_log.h"

#include "rileylink_service.h"
#include "led_mode_handlers.h"


void led_mode_write_handler(uint16_t conn_handle, ble_rileylink_service_t * p_rileylink_service, uint8_t led_state)
{
    if (led_state)
    {
        NRF_LOG_INFO("Received LED ON!");
    }
    else
    {
        NRF_LOG_INFO("Received LED OFF!");
    }
}

