#ifndef SPI_H
#define SPI_H

#define SUBG_RFSPY_SPI_BUFFER_LEN 255
extern uint8_t subg_rfspy_tx_buf[SUBG_RFSPY_SPI_BUFFER_LEN];  /**< TX buffer. */
extern uint8_t subg_rfspy_rx_buf[SUBG_RFSPY_SPI_BUFFER_LEN];  /**< RX buffer. */
extern uint8_t subg_rfspy_tx_len;

void subg_rfspy_spi_init();
void run_command(uint8_t *data, uint8_t data_len);


#endif // SPI_H
