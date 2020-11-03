#ifndef RILEYLINK_CONFIG_H
#define RILEYLINK_CONFIG_H

// Persistent config 

#define CUSTOM_RILEYLINK_NAME_MAX_LEN 100

typedef struct rileylink_config_s
{
    uint16_t version;
    uint8_t custom_name_len;
    uint8_t custom_name[CUSTOM_RILEYLINK_NAME_MAX_LEN];

} rileylink_config_t;

extern rileylink_config_t rileylink_config;

typedef void (*rileylink_config_ready_callback_t)(bool ok);

void rileylink_config_init(rileylink_config_ready_callback_t ready_callback);

#endif // RILEYLINK_CONFIG_H