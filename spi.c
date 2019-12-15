
#include "nrfx_spim.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_delay.h"


#define NRFX_SPIM_SCK_PIN  3
#define NRFX_SPIM_MOSI_PIN 4
#define NRFX_SPIM_MISO_PIN 28
#define NRFX_SPIM_SS_PIN   29
//#define NRFX_SPIM_DCX_PIN  30

#define SPI_INSTANCE 0                                          /**< SPI instance index. */
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */


#define SPI_BUFFER_LEN 255
static uint8_t       m_tx_buf[SPI_BUFFER_LEN];           /**< TX buffer. */
static uint8_t       m_rx_buf[SPI_BUFFER_LEN];  /**< RX buffer. */

void do_spi();

void spim_event_handler(nrfx_spim_evt_t const * p_event,
                       void *                  p_context)
{
    //NRF_LOG_INFO("Transfer completed.");
    if (p_event->type == NRFX_SPIM_EVENT_DONE) {
      if (p_event->xfer_desc.tx_length > 0) {
        NRF_LOG_INFO(" Tx: %d", p_event->xfer_desc.tx_length);
        NRF_LOG_HEXDUMP_INFO(p_event->xfer_desc.p_tx_buffer, p_event->xfer_desc.tx_length);
      }
      if (p_event->xfer_desc.rx_length > 0) {
        NRF_LOG_INFO(" Rx: %d", p_event->xfer_desc.rx_length);
        NRF_LOG_HEXDUMP_INFO(p_event->xfer_desc.p_rx_buffer, p_event->xfer_desc.rx_length);
      }
      // NRF_LOG_INFO("-----------------");
      NRF_LOG_FLUSH();
      spi_xfer_done = true;
    } else {
      NRF_LOG_INFO("Unknown spi event type %d", p_event->type);
      NRF_LOG_FLUSH();
    }
}

void spi_init() {
  nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;
  spi_config.frequency      = 0x00800000UL; // 0x02000000UL = NRF_SPIM_FREQ_125K;
  spi_config.ss_pin         = NRFX_SPIM_SS_PIN;
  spi_config.miso_pin       = NRFX_SPIM_MISO_PIN;
  spi_config.mosi_pin       = NRFX_SPIM_MOSI_PIN;
  spi_config.sck_pin        = NRFX_SPIM_SCK_PIN;
  spi_config.bit_order      = NRF_SPIM_BIT_ORDER_LSB_FIRST;
  spi_config.mode           = NRF_SPIM_MODE_0;  // SCK active high, sample on leading edge of clock
  spi_config.ss_active_high = false;
  APP_ERROR_CHECK(nrfx_spim_init(&spi, &spi_config, spim_event_handler, NULL));

  // Start execution.
  NRF_LOG_INFO("RileyLink 2.0 spi started.");
}

void runCommand(uint8_t command)
{
  do_spi();
}

void do_spi() {
  //int try_count = 3;
  nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(m_tx_buf, 0, m_rx_buf, 0);

  nrf_delay_ms(200);
  // *************** exchange 1

  // Send length
  spi_xfer_done = false;
  memset(m_rx_buf, 0, SPI_BUFFER_LEN);
  m_tx_buf[0] = 0x99;   // marker
  m_tx_buf[1] = 1;      // length of command
  xfer_desc.tx_length = 2;
  xfer_desc.rx_length = 2;
  APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));
  while (!spi_xfer_done)
  {
      __WFE();
  }

  // Send command
  spi_xfer_done = false;
  memset(m_rx_buf, 0, SPI_BUFFER_LEN);
  m_tx_buf[0] = 2;      // Get version command
  xfer_desc.tx_length = 1;
  xfer_desc.rx_length = 1;
  APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));
  while (!spi_xfer_done)
  {
      __WFE();
  }

  nrf_delay_ms(200);


  // *************** exchange 2

  // Get response length
  spi_xfer_done = false;
  memset(m_rx_buf, 0, SPI_BUFFER_LEN);
  m_tx_buf[0] = 0x99;   // marker
  m_tx_buf[1] = 0;      // no data to send
  xfer_desc.tx_length = 2;
  xfer_desc.rx_length = 2;
  APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));
  while (!spi_xfer_done)
  {
      __WFE();
  }

  int response_length = m_rx_buf[1];

  if (response_length > 0) {
    NRF_LOG_INFO("Expecting response! %d", response_length);
    // Get response data
    spi_xfer_done = false;
    memset(m_rx_buf, 0, SPI_BUFFER_LEN);
    xfer_desc.tx_length = 0;
    xfer_desc.rx_length = response_length;
    APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));
    while (!spi_xfer_done)
    {
        __WFE();
    }
  }

}
