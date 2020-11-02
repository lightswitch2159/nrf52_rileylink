
#include "nrf_log.h"
#include "app_error.h"

#include "rileylink_service.h"
#include "led_mode_handlers.h"
#include "subg_rfspy_spi.h"

static ble_rileylink_service_t *m_rileylink_service;

void data_relay_init(ble_rileylink_service_t * p_rileylink_service) {
    m_rileylink_service = p_rileylink_service;
}

void data_relay_ble_write_handler(const uint8_t *data, uint16_t length)
{
    NRF_LOG_INFO("Data received via BLE: %d bytes.", length);
    if (length >= 2) {
        subg_rfspy_spi_run_command(data+1, length-1);
    }
}

void data_relay_spi_response_handler(const uint8_t *data, uint8_t length) {
    uint32_t   err_code;

    NRF_LOG_INFO("Data received via SPI: %d bytes.", length);
    err_code = ble_rileylink_service_send_data(m_rileylink_service, data, length);
    APP_ERROR_CHECK(err_code);
}

