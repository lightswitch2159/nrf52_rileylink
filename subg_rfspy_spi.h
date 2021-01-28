#ifndef SPI_H
#define SPI_H

#define BLE_RILEYLINK_SERVICE_DEF(_name)                                                                    \
static ble_rileylink_service_t _name;                                                                       \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                         \
                     BLE_RILEYLINK_SERVICE_BLE_OBSERVER_PRIO,                                                     \
                     ble_rileylink_service_on_ble_evt, &_name)


#define SUBG_RFSPY_RECEIVE_INTERRUPT_PIN 7

#define SUBG_RFSPY_SPI_BUFFER_LEN 255

#define CC1110_RESET_PIN   30


typedef void (subg_rfspy_spi_response_handler_t) (const uint8_t *data, uint8_t len);

void subg_rfspy_spi_init(subg_rfspy_spi_response_handler_t response_handler);
void subg_rfspy_spi_run_command(const uint8_t *data, uint8_t data_len);
void subg_rfspy_spi_data_available();


#endif // SPI_H
