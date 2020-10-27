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



// RL Service UUID = 02350001-99c5-4197-b856-69219c2a3846
#define BLE_UUID_RILEYLINK_SERVICE_BASE_UUID    {0x46, 0x38, 0x2a, 0x9c, 0x21, 0x69, 0x56, 0xb8, \
                                                 0x97, 0x41, 0xc5, 0x99, 0x00, 0x00, 0x35, 0x02}

#define LOCAL_CHAR_UUID                 0x1234                                      /**< Proprietary UUID for local characteristic. */
#define APP_CFG_CHAR_LEN                20                                          /**< Size of the characteristic value being notified (in bytes). */
#define APP_REPORT_CHAR_LEN           8                                             /**< Size of the characteristic value being notified (in bytes). */
#define APP_CTRLPT_CHAR_LEN           8                                             /**< Size of the control point characteristic value being notified (in bytes). */

extern uint8_t rileylink_service_uuid_type;
extern uint16_t rileylink_service_handle;

// Service & characteristics UUIDs
#define BLE_UUID_RILEYLINK_SERVICE_UUID        0x0001
#define BLE_UUID_RILEYLINK_DATA_UUID           0x0002
#define BLE_UUID_RILEYLINK_RESPONSE_COUNT_UUID 0x0003
#define BLE_UUID_RILEYLINK_TIMER_UUID          0x0004
#define BLE_UUID_RILEYLINK_CUSTOM_NAME_UUID    0x0005
#define BLE_UUID_RILEYLINK_VERSION_UUID        0x0006
#define BLE_UUID_RILEYLINK_LED_MODE_UUID       0x0007

// Characteristics:
// Data:           0002
// Response count: 0003
// Timer tick:     0004
// Custom name:    0005
// Version:        0006
// LED Mode:       0007

// Forward declaration of the custom_service_t type.
typedef struct ble_rileylink_service_s ble_rileylink_service_t;

typedef void (*ble_rileylink_service_led_mode_write_handler_t) (uint16_t conn_handle, ble_rileylink_service_t * p_rileylink_service, uint8_t new_state);

/** @brief LED Service init structure. This structure contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_rileylink_service_led_mode_write_handler_t led_mode_write_handler; /**< Event handler to be called when the LED Characteristic is written. */
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
    ble_gatts_char_handles_t            led_mode_char_handles;
    ble_rileylink_service_led_mode_write_handler_t led_mode_write_handler;

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

#endif //RILEYLINK_SERVICE_H
