
#include "rileylink_service.h"

#include "nordic_common.h"
#include "sdk_errors.h"
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_error.h"
#include "ble.h"
#include "ble_err.h"
#include "nrf_log.h"
#include "app_error.h"


uint8_t rileylink_service_uuid_type;
uint16_t rileylink_service_handle;

static uint8_t                        m_char_value[APP_REPORT_CHAR_LEN];            /**< Value of the characteristic that will be sent as a notification to the central. */
static ble_gatts_char_handles_t       m_char_handles;                               /**< Handles of the characteristic (as provided by the BLE stack). */

/**@brief 128-bit UUID base List. */
static ble_uuid128_t const m_base_uuid128 = {RILEYLINK_UUID_BASE};

static void char_add(const uint8_t uuid_type)
{
    ret_code_t          err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          char_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    char_uuid.type = uuid_type;
    char_uuid.uuid = LOCAL_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &char_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = APP_CFG_CHAR_LEN;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = APP_CFG_CHAR_LEN;
    attr_char_value.p_value   = m_char_value;

    err_code = sd_ble_gatts_characteristic_add(rileylink_service_handle,
                                               &char_md,
                                               &attr_char_value,
                                               &m_char_handles);
    APP_ERROR_CHECK(err_code);
}


uint32_t rileylink_service_init() {

  ret_code_t            err_code;
  ble_uuid_t            service_uuid;

  service_uuid.uuid = RILEYLINK_UUID_SERVICE;

  /**@snippet [Adding proprietary Service to the SoftDevice] */
  // Add a custom base UUID.
  err_code = sd_ble_uuid_vs_add(&m_base_uuid128, &rileylink_service_uuid_type);
  VERIFY_SUCCESS(err_code);

  service_uuid.type = rileylink_service_uuid_type;

  // Add the service.
  err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                      &service_uuid,
                                      &rileylink_service_handle);
  /**@snippet [Adding proprietary Service to the SoftDevice] */
  VERIFY_SUCCESS(err_code);

  NRF_LOG_INFO("Initialized RileyLink service");

  char_add(service_uuid.type);

  return err_code;


  // Add the RX Characteristic.
  // memset(&add_char_params, 0, sizeof(add_char_params));
  // add_char_params.uuid                     = BLE_UUID_NUS_RX_CHARACTERISTIC;
  // add_char_params.uuid_type                = p_nus->uuid_type;
  // add_char_params.max_len                  = BLE_NUS_MAX_RX_CHAR_LEN;
  // add_char_params.init_len                 = sizeof(uint8_t);
  // add_char_params.is_var_len               = true;
  // add_char_params.char_props.write         = 1;
  // add_char_params.char_props.write_wo_resp = 1;
  //
  // add_char_params.read_access  = SEC_OPEN;
  // add_char_params.write_access = SEC_OPEN;
  //
  // err_code = characteristic_add(p_nus->service_handle, &add_char_params, &p_nus->rx_handles);
  // if (err_code != NRF_SUCCESS)
  // {
  //     return err_code;
  // }
  //
  // // Add the TX Characteristic.
  // /**@snippet [Adding proprietary characteristic to the SoftDevice] */
  // memset(&add_char_params, 0, sizeof(add_char_params));
  // add_char_params.uuid              = BLE_UUID_NUS_TX_CHARACTERISTIC;
  // add_char_params.uuid_type         = p_nus->uuid_type;
  // add_char_params.max_len           = BLE_NUS_MAX_TX_CHAR_LEN;
  // add_char_params.init_len          = sizeof(uint8_t);
  // add_char_params.is_var_len        = true;
  // add_char_params.char_props.notify = 1;
  //
  // add_char_params.read_access       = SEC_OPEN;
  // add_char_params.write_access      = SEC_OPEN;
  // add_char_params.cccd_write_access = SEC_OPEN;
  //
  // return characteristic_add(p_nus->service_handle, &add_char_params, &p_nus->tx_handles);
  // /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}
