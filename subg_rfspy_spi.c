
#include "nrfx_spim.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_delay.h"

#include "subg_rfspy_spi.h"

#define NRFX_SPIM_SCK_PIN  3
#define NRFX_SPIM_MOSI_PIN 4
#define NRFX_SPIM_MISO_PIN 28
#define NRFX_SPIM_SS_PIN   29
//#define NRFX_SPIM_DCX_PIN  30

#define SPI_INSTANCE 0                                            /**< SPI instance index. */
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

uint8_t subg_rfspy_tx_buf[SUBG_RFSPY_SPI_BUFFER_LEN];  /**< TX buffer. */
uint8_t subg_rfspy_rx_buf[SUBG_RFSPY_SPI_BUFFER_LEN];  /**< RX buffer. */
uint8_t subg_rfspy_tx_len;

static uint8_t size_tx_buf[2];  /**< Size exchange TX buffer. */
static uint8_t size_rx_buf[2];  /**< Size exchange RX buffer. */

static enum State{Idle,Size,Tx,Wait,Rx} state;

static void size_exchange();
static void send_data();
static void receive_data();

void do_spi();

void spim_event_handler(nrfx_spim_evt_t const * p_event,
                       void *                  p_context)
{
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
    } else {
      NRF_LOG_INFO("Unknown spi event type %d", p_event->type);
      NRF_LOG_FLUSH();
    }

    switch (state) {
    case Size:
      state = Tx;
      send_data();
      break;
    case Tx:
      state = Wait;
      size_exchange();
      break;
    case Wait:
      if (size_rx_buf[1] > 0) {
        state = Rx;
        receive_data();
      } else {
        size_exchange();
      }
      break;
    case Rx:
      state = Idle;
      NRF_LOG_INFO("Xfer finished");
      break;
    default:
      NRF_LOG_INFO("finished spi event during idle???");
    }
}

void subg_rfspy_spi_init() {
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

  state = Idle;
}

void run_command(uint8_t command)
{
  if (state != Idle) {
    NRF_LOG_INFO("Skipped command: busy");
    return;
  }
  state = Size;
  subg_rfspy_tx_buf[0] = command;
  subg_rfspy_tx_len = 1;

  size_exchange();
}

void size_exchange() {
  // Send length
  nrfx_spim_xfer_desc_t size_xfer_desc = NRFX_SPIM_XFER_TRX(size_tx_buf, 0, size_rx_buf, 0);
  spi_xfer_done = false;
  size_tx_buf[0] = 0x99;                   // marker
  size_tx_buf[1] = subg_rfspy_tx_len;      // length of command
  size_xfer_desc.tx_length = 2;
  size_xfer_desc.rx_length = 2;
  APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &size_xfer_desc, 0));
}

void send_data() {
  nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(subg_rfspy_tx_buf, 0, subg_rfspy_rx_buf, 0);
  spi_xfer_done = false;
  xfer_desc.tx_length = size_tx_buf[1];
  xfer_desc.rx_length = size_rx_buf[1];
  subg_rfspy_tx_len = 0;
  APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));
}

void receive_data() {
  nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(subg_rfspy_tx_buf, 0, subg_rfspy_rx_buf, 0);
  spi_xfer_done = false;
  xfer_desc.tx_length = size_tx_buf[1];
  xfer_desc.rx_length = size_rx_buf[1];
  APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));
}
