#ifndef RILEYLINK_SERVICE_H
#define RILEYLINK_SERVICE_H

#include <stdint.h>

// RL Service UUID = 0235733b-99c5-4197-b856-69219c2a3845

#define RILEYLINK_UUID_SERVICE 0x733b
#define RILEYLINK_UUID_BASE    {0x45, 0x38, 0x2a, 0x9c, 0x21, 0x69, 0x56, 0xb8, \
                                0x97, 0x41, 0xc5, 0x99, 0x00, 0x00, 0x35, 0x02}

#define LOCAL_CHAR_UUID                 0x1234                                        /**< Proprietary UUID for local characteristic. */
#define APP_CFG_CHAR_LEN                20                                            /**< Size of the characteristic value being notified (in bytes). */
#define APP_REPORT_CHAR_LEN           8                                             /**< Size of the characteristic value being notified (in bytes). */
#define APP_CTRLPT_CHAR_LEN           8                                             /**< Size of the control point characteristic value being notified (in bytes). */

// Version UUID 30d99dc9-7c91-4295-a051-0a104d238cf2

extern uint8_t rileylink_service_uuid_type;
extern uint16_t rileylink_service_handle;

uint32_t rileylink_service_init();


#endif //RILEYLINK_SERVICE_H
