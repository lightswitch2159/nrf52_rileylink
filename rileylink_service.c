
#include "nordic_common.h"
#include "sdk_errors.h"
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_error.h"
#include "ble.h"
#include "ble_err.h"
#include "nrf_log.h"
#include "nrf_ble_qwr.h"
#include "app_error.h"

#include "rileylink_service.h"
#include "rileylink_config.h"

static const uint8_t LEDModeCharName[] = "LED Mode";
static const uint8_t DataCharName[] = "Data";
static const uint8_t ResponseCountCharName[] = "Response Count";
static const uint8_t VersionCharName[] = "Version";
static const uint8_t TimerTickCharName[] = "Timer Tick";
static const uint8_t CustomNameCharName[] = "Custom Name";
static uint8_t FirmwareVersion[] = "nrf52_rileylink 1.0";

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_rileylink_service  RileyLink service structure.
 * @param[in]   p_ble_evt      Event received from the BLE stack.
 */
static void on_connect(ble_rileylink_service_t * p_rileylink_service, ble_evt_t const * p_ble_evt)
{
    p_rileylink_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_bas       LED service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_rileylink_service_t * p_rileylink_service, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_rileylink_service->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void on_custom_name_update(ble_rileylink_service_t * p_rileylink_service, const uint8_t *name, uint16_t len)
{
    if (len > CUSTOM_RILEYLINK_NAME_MAX_LEN) {
        len = CUSTOM_RILEYLINK_NAME_MAX_LEN;
    }
    // Data is already in rileylink_config, because we are using BLE_GATTS_VLOC_USER and pointing to the memory in rileylink_config.
    rileylink_config.custom_name_len = len;
    rileylink_config_save();

    if (p_rileylink_service->named_changed_callback != NULL) {
        p_rileylink_service->named_changed_callback();
    }
}

/**@brief Function for handling the Write event.
 *
 * @param[in] p_led_service   LED Service structure.
 * @param[in] p_ble_evt       Event received from the BLE stack.
 */
static void on_write(ble_rileylink_service_t * p_rileylink_service, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (   (p_evt_write->handle == p_rileylink_service->led_mode_char_handles.value_handle)
        && (p_evt_write->len == 1)
        && (p_rileylink_service->led_mode_write_handler != NULL))
    {
        p_rileylink_service->led_mode_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_rileylink_service, p_evt_write->data[0]);
    }
    else if ((p_evt_write->handle == p_rileylink_service->data_char_handles.value_handle)
          && (p_rileylink_service->data_write_handler != NULL))
    {
        p_rileylink_service->data_write_handler(p_evt_write->data, p_evt_write->len);
    }
    else if ((p_evt_write->handle == p_rileylink_service->custom_name_char_handles.value_handle)
          && (p_rileylink_service->data_write_handler != NULL))
    {
        NRF_LOG_DEBUG("Name update!");
        on_custom_name_update(p_rileylink_service, p_evt_write->data, p_evt_write->len);
    }
    else 
    {
        NRF_LOG_DEBUG("Unhandled write");
    }
}

/**@brief Function for adding the LED mode characteristic.
 *
 */
static uint32_t led_mode_char_add(ble_rileylink_service_t * p_rileylink_service)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;
    ble_uuid_t          ble_uuid;

    memset(&char_md, 0, sizeof(char_md));
    memset(&attr_md, 0, sizeof(attr_md));
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.p_char_user_desc         = LEDModeCharName;
    char_md.char_user_desc_size      = sizeof(LEDModeCharName);
    char_md.char_user_desc_max_size  = sizeof(LEDModeCharName);

    // Define the LED Nide Characteristic UUID
    ble_uuid.type = p_rileylink_service->uuid_type;
    ble_uuid.uuid = BLE_UUID_RILEYLINK_LED_MODE_UUID;

    // Set permissions on the Characteristic value
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    // Attribute Metadata settings
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;

    // Attribute Value settings
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = NULL;

    return sd_ble_gatts_characteristic_add(p_rileylink_service->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_rileylink_service->led_mode_char_handles);
}

/**@brief Function for adding the Data characteristic.
 *
 */
static uint32_t data_char_add(ble_rileylink_service_t * p_rileylink_service)
{
    uint32_t err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;
    ble_uuid_t          ble_uuid;

    memset(&char_md, 0, sizeof(char_md));
    memset(&attr_md, 0, sizeof(attr_md));
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.p_char_user_desc         = DataCharName;
    char_md.char_user_desc_size      = sizeof(DataCharName);
    char_md.char_user_desc_max_size  = sizeof(DataCharName);

    // Define the DATA Characteristic UUID
    ble_uuid128_t base_uuid = {BLE_UUID_RILEYLINK_DATA_BASE_UUID};
    uint8_t uuid_type;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    ble_uuid.type = uuid_type;
    ble_uuid.uuid = BLE_UUID_RILEYLINK_DATA_UUID;

    // Set permissions on the Characteristic value
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    // Attribute Metadata settings
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.vlen       = 1;

    // Attribute Value settings
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.max_len      = BLE_RILEYLINK_DATA_MAX_LENGTH;
    attr_char_value.p_value      = NULL;

    err_code = sd_ble_gatts_characteristic_add(p_rileylink_service->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_rileylink_service->data_char_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Response Count characteristic.
 *
 */
static uint32_t response_count_char_add(ble_rileylink_service_t * p_rileylink_service)
{
    uint32_t err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;
    ble_uuid_t          ble_uuid;

    memset(&char_md, 0, sizeof(char_md));
    memset(&attr_md, 0, sizeof(attr_md));
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    char_md.char_props.read          = 1;
    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = ResponseCountCharName;
    char_md.char_user_desc_size      = sizeof(ResponseCountCharName);
    char_md.char_user_desc_max_size  = sizeof(ResponseCountCharName);

    // Define the Response Count Characteristic UUID
    ble_uuid128_t base_uuid = {BLE_UUID_RILEYLINK_RESPONSE_COUNT_BASE_UUID};
    uint8_t uuid_type;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    ble_uuid.type = uuid_type;
    ble_uuid.uuid = BLE_UUID_RILEYLINK_RESPONSE_COUNT_UUID;

    // Set permissions on the Characteristic value
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    // Attribute Metadata settings
    attr_md.vloc       = BLE_GATTS_VLOC_USER;

    // Attribute Value settings
    p_rileylink_service->response_count = 0;
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 1;
    attr_char_value.max_len      = 1;
    attr_char_value.p_value      = &(p_rileylink_service->response_count);

    return sd_ble_gatts_characteristic_add(p_rileylink_service->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_rileylink_service->response_count_char_handles);
}

/**@brief Function for adding the Version characteristic.
 *
 */
static uint32_t version_char_add(ble_rileylink_service_t * p_rileylink_service)
{
    uint32_t err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;
    ble_uuid_t          ble_uuid;

    memset(&char_md, 0, sizeof(char_md));
    memset(&attr_md, 0, sizeof(attr_md));
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = VersionCharName;
    char_md.char_user_desc_size      = sizeof(VersionCharName);
    char_md.char_user_desc_max_size  = sizeof(VersionCharName);

    // Define the Version Characteristic UUID
    ble_uuid128_t base_uuid = {BLE_UUID_RILEYLINK_VERSION_BASE_UUID};
    uint8_t uuid_type;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    ble_uuid.type = uuid_type;
    ble_uuid.uuid = BLE_UUID_RILEYLINK_VERSION_UUID;

    // Set permissions on the Characteristic value
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    // Attribute Metadata settings
    attr_md.vloc       = BLE_GATTS_VLOC_USER;

    // Attribute Value settings
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(FirmwareVersion);
    attr_char_value.max_len      = sizeof(FirmwareVersion);
    attr_char_value.p_value      = (uint8_t*)FirmwareVersion;

    return sd_ble_gatts_characteristic_add(p_rileylink_service->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_rileylink_service->version_char_handles);
}

/**@brief Function for adding the Timer Tick characteristic.
 *
 */
static uint32_t timer_tick_char_add(ble_rileylink_service_t * p_rileylink_service)
{
    uint32_t err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;
    ble_uuid_t          ble_uuid;

    memset(&char_md, 0, sizeof(char_md));
    memset(&attr_md, 0, sizeof(attr_md));
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    char_md.char_props.read          = 1;
    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = TimerTickCharName;
    char_md.char_user_desc_size      = sizeof(TimerTickCharName);
    char_md.char_user_desc_max_size  = sizeof(TimerTickCharName);

    // Define the Timer Tick Characteristic UUID
    ble_uuid128_t base_uuid = {BLE_UUID_RILEYLINK_TIMER_TICK_BASE_UUID};
    uint8_t uuid_type;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    ble_uuid.type = uuid_type;
    ble_uuid.uuid = BLE_UUID_RILEYLINK_TIMER_TICK_UUID;

    // Set permissions on the Characteristic value
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    // Attribute Metadata settings
    attr_md.vloc       = BLE_GATTS_VLOC_USER;

    // Attribute Value settings
    p_rileylink_service->response_count = 0;
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 1;
    attr_char_value.max_len      = 1;
    attr_char_value.p_value      = &(p_rileylink_service->timer_tick_count);

    return sd_ble_gatts_characteristic_add(p_rileylink_service->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_rileylink_service->timer_tick_char_handles);
}

/**@brief Function for adding the Custom Name characteristic.
 *
 */
static uint32_t custom_name_char_add(ble_rileylink_service_t * p_rileylink_service)
{
    uint32_t err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;
    ble_uuid_t          ble_uuid;

    memset(&char_md, 0, sizeof(char_md));
    memset(&attr_md, 0, sizeof(attr_md));
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.p_char_user_desc         = CustomNameCharName;
    char_md.char_user_desc_size      = sizeof(CustomNameCharName);
    char_md.char_user_desc_max_size  = sizeof(CustomNameCharName);

    // Define the DATA Characteristic UUID
    ble_uuid128_t base_uuid = {BLE_UUID_RILEYLINK_CUSTOM_NAME_BASE_UUID};
    uint8_t uuid_type;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    ble_uuid.type = uuid_type;
    ble_uuid.uuid = BLE_UUID_RILEYLINK_CUSTOM_NAME_UUID;

    // Set permissions on the Characteristic value
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    // Attribute Metadata settings
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.vlen       = 1;

    // Attribute Value settings
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.max_len      = CUSTOM_RILEYLINK_NAME_MAX_LEN;
    attr_char_value.p_value      = rileylink_config.custom_name;
    attr_char_value.init_len     = rileylink_config.custom_name_len;

    err_code = sd_ble_gatts_characteristic_add(p_rileylink_service->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_rileylink_service->custom_name_char_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }



    return NRF_SUCCESS;
}


uint32_t ble_rileylink_service_init(ble_rileylink_service_t * p_rileylink_service, const ble_rileylink_service_init_t * p_rileylink_service_init, ble_rileylink_service_name_changed_callback_t named_changed_callback)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_rileylink_service->conn_handle = BLE_CONN_HANDLE_INVALID;

    // Initialize service structure.
    p_rileylink_service->led_mode_write_handler = p_rileylink_service_init->led_mode_write_handler;
    p_rileylink_service->data_write_handler = p_rileylink_service_init->data_write_handler;
    p_rileylink_service->named_changed_callback = named_changed_callback;

    // Add service UUID
    ble_uuid128_t base_uuid = {BLE_UUID_RILEYLINK_SERVICE_BASE_UUID};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_rileylink_service->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Set up the UUID for the service (base + service-specific)
    ble_uuid.type = p_rileylink_service->uuid_type;
    ble_uuid.uuid = BLE_UUID_RILEYLINK_SERVICE_UUID;

    p_rileylink_service->response_count = 0;

    // Set up and add the service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_rileylink_service->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the different characteristics in the service:
    err_code = led_mode_char_add(p_rileylink_service);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = data_char_add(p_rileylink_service);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = response_count_char_add(p_rileylink_service);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = version_char_add(p_rileylink_service);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = timer_tick_char_add(p_rileylink_service);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = custom_name_char_add(p_rileylink_service);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

void ble_rileylink_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_rileylink_service_t * p_rileylink_service = (ble_rileylink_service_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_rileylink_service, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            NRF_LOG_DEBUG("GATTS Write");
            on_write(p_rileylink_service, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_rileylink_service, p_ble_evt);
            break;

        default:
            NRF_LOG_DEBUG("Unhandled BLE event: 0x%x.", p_ble_evt->header.evt_id);
            // No implementation needed.
            break;
    }
}

uint32_t ble_rileylink_service_send_data(ble_rileylink_service_t * p_rileylink_service, const uint8_t *data, uint8_t length) {
    ble_gatts_value_t new_value;
    uint32_t err_code;
    uint16_t response_count_length;
    ble_gatts_hvx_params_t hvx_params;

    memset(&new_value, 0, sizeof(new_value));
    new_value.len     = length;
    new_value.offset  = 0;
    new_value.p_value = (uint8_t*)data;
    err_code = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID, p_rileylink_service->data_char_handles.value_handle, &new_value);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    p_rileylink_service->response_count++;

    if (p_rileylink_service->conn_handle != BLE_CONN_HANDLE_INVALID) {
        response_count_length = 1;
        memset(&hvx_params, 0, sizeof(hvx_params));
        hvx_params.handle = p_rileylink_service->response_count_char_handles.value_handle;
        hvx_params.p_data = &(p_rileylink_service->response_count);
        hvx_params.p_len = &response_count_length;
        hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

        err_code = sd_ble_gatts_hvx(p_rileylink_service->conn_handle, &hvx_params);
        // NRF_ERROR_INVALID_STATE means client has not subscribed to this notification
        if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE) {
            NRF_LOG_DEBUG("sd_ble_gatts_hvx error: 0x%x", err_code);
            return err_code;
        }
    }
    return NRF_SUCCESS;
}

void ble_rileylink_service_timer_tick(ble_rileylink_service_t * p_rileylink_service) {
    uint32_t err_code;

    p_rileylink_service->timer_tick_count++;

    if (p_rileylink_service->conn_handle != BLE_CONN_HANDLE_INVALID) {
        ble_gatts_hvx_params_t hvx_params;
        uint16_t timer_tick_length = 1;
        memset(&hvx_params, 0, sizeof(hvx_params));
        hvx_params.handle = p_rileylink_service->timer_tick_char_handles.value_handle;
        hvx_params.p_data = &(p_rileylink_service->timer_tick_count);
        hvx_params.p_len = &timer_tick_length;
        hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

        err_code = sd_ble_gatts_hvx(p_rileylink_service->conn_handle, &hvx_params);
        // NRF_ERROR_INVALID_STATE means client has not subscribed to this notification
        if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE) {
            NRF_LOG_DEBUG("sd_ble_gatts_hvx error: 0x%x", err_code);
        }
    }
}