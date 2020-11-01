#ifndef RILEYLINK_SERVICE_H
#define RILEYLINK_SERVICE_H

#include <stdint.h>
#include "nrf_sdh_ble.h"


#define BLE_RILEYLINK_SERVICE_BLE_OBSERVER_PRIO 2

#define BLE_RILEYLINK_SERVICE_DEF(_name)                                                                    \
static ble_rileylink_service_t _name;                                                                       \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                         \
                     BLE_RILEYLINK_SERVICE_BLE_OBSERVER_PRIO,                                                     \
                     ble_rileylink_service_on_ble_evt, &_name)



// RL Service UUID = 02350001-99c5-4197-b856-69219c2a3845
//                   0235733b-99c5-4197-b856-69219c2a3845
#define BLE_UUID_RILEYLINK_SERVICE_BASE_UUID    {0x45, 0x38, 0x2a, 0x9c, 0x21, 0x69, 0x56, 0xb8, \
                                                 0x97, 0x41, 0xc5, 0x99, 0x3b, 0x73, 0x35, 0x02}
#define BLE_UUID_RILEYLINK_SERVICE_UUID        0x733b

#define LOCAL_CHAR_UUID                 0x1234                                      /**< Proprietary UUID for local characteristic. */
#define APP_CFG_CHAR_LEN                20                                          /**< Size of the characteristic value being notified (in bytes). */
#define APP_REPORT_CHAR_LEN           8                                             /**< Size of the characteristic value being notified (in bytes). */
#define APP_CTRLPT_CHAR_LEN           8                                             /**< Size of the control point characteristic value being notified (in bytes). */

extern uint8_t rileylink_service_uuid_type;
extern uint16_t rileylink_service_handle;

// Characteristics UUIDs

// Data - c842e849-5028-42e2-867c-016adada9155
#define BLE_UUID_RILEYLINK_DATA_BASE_UUID {0x55, 0x91, 0xda, 0xda, 0x6a, 0x01, 0x7c, 0x86, \
                                           0xe2, 0x42, 0x28, 0x50, 0x49, 0xe8, 0x42, 0xc8}
#define BLE_UUID_RILEYLINK_DATA_UUID 0xe849

// Response Count - 6e6c7910-b89e-43a5-a0fe-50c5e2b81f4a
#define BLE_UUID_RILEYLINK_RESPONSE_COUNT_BASE_UUID {0x4a, 0x1f, 0xb8, 0xe2, 0xc5, 0x50, 0xfe, 0xa0, \
                                                     0xa5, 0x43, 0x9e, 0xb8, 0x10, 0x79, 0x6c, 0x6e}
#define BLE_UUID_RILEYLINK_RESPONSE_COUNT_UUID 0x7910

// Timer - 6e6c7910-b89e-43a5-78af-50c5e2b86f7e
#define BLE_UUID_RILEYLINK_TIMER_BASE_UUID {0x7e, 0x6f, 0xb8, 0xe2, 0xc5, 0x50, 0xaf, 0x78, \
                                            0xa5, 0x43, 0x9e, 0xb8, 0x10, 0x79, 0x6c, 0x6e}
#define BLE_UUID_RILEYLINK_TIMER_UUID 0x7910

// Custom Name - d93b2af0-1e28-11e4-8c21-0800200c9a66
#define BLE_UUID_RILEYLINK_CUSTOM_NAME_BASE_UUID {0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x21, 0x8c, \
                                                  0xe4, 0x11, 0x28, 0x1e, 0xf0, 0x2a, 0x3b, 0xd9}
#define BLE_UUID_RILEYLINK_CUSTOM_NAME_UUID 0x2af0

// Version - 30d99dc9-7c91-4295-a051-0a104d238cf2
#define BLE_UUID_RILEYLINK_VERSION_BASE_UUID {0xf2, 0x8c, 0x23, 0x4d, 0x10, 0x0a, 0x51, 0xa0, \
                                              0x95, 0x42, 0x91, 0x7c, 0xc9, 0x9d, 0xd9, 0x30}
#define BLE_UUID_RILEYLINK_VERSION_UUID 0x9dc9

// LED Mode - c6d84241-f1a7-4f9c-a25f-fce16732f14e
#define BLE_UUID_RILEYLINK_LED_MODE_BASE_UUID {0x4e, 0xf1, 0x32, 0x67, 0xe1, 0xfc, 0x5f, 0xa2, \
                                               0x9c, 0x4f, 0xa7, 0xf1, 0x41, 0x42, 0xd8, 0xc6}
#define BLE_UUID_RILEYLINK_LED_MODE_UUID 0x4241

// Forward declaration of the custom_service_t type.
typedef struct ble_rileylink_service_s ble_rileylink_service_t;

typedef void (*ble_rileylink_service_led_mode_write_handler_t) (uint16_t conn_handle, ble_rileylink_service_t * p_rileylink_service, uint8_t new_state);
typedef void (*ble_rileylink_service_data_write_handler_t) (uint16_t conn_handle, ble_rileylink_service_t * p_rileylink_service, const uint8_t *data, uint16_t length);

/** @brief LED Service init structure. This structure contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_rileylink_service_led_mode_write_handler_t led_mode_write_handler; /**< Event handler to be called when the LED Characteristic is written. */
    ble_rileylink_service_data_write_handler_t data_write_handler; /**< Event handler to be called when the DATA Characteristic is written. */
} ble_rileylink_service_init_t;

/**@brief RileyLink Service structure.
 *        This contains various status information
 *        for the service.
 */
typedef struct ble_rileylink_service_s
{
    uint16_t                            conn_handle;
    uint16_t                            service_handle;
    uint8_t                             uuid_type;
    uint8_t                             response_count;
    ble_gatts_char_handles_t            led_mode_char_handles;
    ble_gatts_char_handles_t            data_char_handles;
    ble_gatts_char_handles_t            response_count_handles;
    ble_gatts_char_handles_t            version_handles;
    ble_rileylink_service_led_mode_write_handler_t led_mode_write_handler;
    ble_rileylink_service_data_write_handler_t data_write_handler;

} ble_rileylink_service_t;

// Function Declarations

/**@brief Function for initializing the RileyLink Service.
 *
 * @param[out]  p_rileylink_service  RileyLink Service structure. This structure will have to be supplied by
 *                                the application. It will be initialized by this function, and will later
 *                                be used to identify this particular service instance.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_rileylink_service_init(ble_rileylink_service_t * p_rileylink_service, const ble_rileylink_service_init_t * p_rileylink_service_init);

/**@brief Function for handling the application's BLE stack events.
 *
 * @details This function handles all events from the BLE stack that are of interest to the RileyLink Service.
 *
 * @param[in] p_ble_evt  Event received from the BLE stack.
 * @param[in] p_context  LED Service structure.
 */
void ble_rileylink_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/* Send data via BLE DATA characteristic */
uint32_t ble_rileylink_service_send_data(ble_rileylink_service_t * p_rileylink_service, const uint8_t *data, uint8_t length);


#endif //RILEYLINK_SERVICE_H
