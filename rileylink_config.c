
#include "fds.h"

#include "rileylink_config.h"

#define RL_CONFIG_FILE_ID 0x1001
#define RL_CONFIG_KEY 0x1000


__ALIGN(4) rileylink_config_t rileylink_config;

__ALIGN(4) rileylink_config_t rileylink_config_buffer;

volatile static bool isStoring = false;
volatile static bool updatedWhileStoring = false;

rileylink_config_ready_callback_t m_ready_callback;

void save_rileylink_config()
{
    if (isStoring) {
      updatedWhileStoring = true;
      return;
    }

    memcpy(rileylink_config_buffer, rileylink_config, sizeof(rileylink_config));

    fds_record_desc_t  record_desc;
    fds_record_t       record;

    memset(&record, 0, sizeof(record));

    record.file_id           = RL_CONFIG_FILE_ID;
    record.key               = RL_CONFIG_KEY;
    record.data.length_words = sizeof(rileylink_config) / 4;
    record.data.p_data       = (uint8_t *) &rileylink_config_buffer;

    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

    ret_code_t rc = fds_record_find(RL_CONFIG_FILE_ID, RL_CONFIG_KEY , &desc, &tok)

    if (rc == FDS_SUCCESS)
    {
        NRF_LOG_INFO("Updating %d bytes to fds", record.data.length_words);
        err_code = fds_record_update(&desc, &record);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        NRF_LOG_INFO("Writing %d bytes to fds", record.data.length_words);
        err_code = fds_record_write(&record_desc, &record);
        APP_ERROR_CHECK(err_code);
    }
}

static void read_rileylink_config()
{
    fds_record_desc_t  record_desc;
    fds_record_t       record;

    memset(&record, 0, sizeof(record));

    record.file_id           = RILEYLINK_CONFIG_FILE_ID;
    record.key               = RILEYLINK_NAME_RECORD_ID;
    record.data.length_words = sizeof(rileylink_config) / 4;
    record.data.p_data       = (uint8_t *) &rileylink_config_buffer;

    NRF_LOG_INFO("reading %d bytes from fds", record.data.length_words);

    err_code = fds_record (&record_desc, &record);
    APP_ERROR_CHECK(err_code);
}

static void flash_callback(fds_evt_t const * p_evt)
{
    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if(p_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_INFO("FDS initialized");
                if (m_ready_callback != NULL) {
                  m_ready_callback(true);
                }
            }
            else 
            {
                NRF_LOG_INFO("FDS initialization failed");
                if (m_ready_callback != NULL) {
                  m_ready_callback(false);
                }
            }
            break;
            
        case FDS_EVT_WRITE:
            NRF_LOG_INFO("FDS_EVT_WRITE");
            if(p_evt->write.record_key == RILEYLINK_CONFIG_RECORD_ID && p_evt->write.file_id == RILEYLINK_CONFIG_FILE_ID)
            {
                NRF_LOG_INFO("FDS_EVT_WRITE config");
                config_save_finished(p_evt->result == FDS_SUCCESS)
            }
            break;
        case FDS_EVT_UPDATE:
            if(p_evt->write.record_key == RILEYLINK_CONFIG_RECORD_ID && p_evt->write.file_id == RILEYLINK_CONFIG_FILE_ID)
            {
                NRF_LOG_INFO("FDS_EVT_UPDATE config");
                config_save_finished(p_evt->result == FDS_SUCCESS)
            }
            break;
        case FDS_EVT_DEL_RECORD:
        case FDS_EVT_DEL_FILE:
        case FDS_EVT_GC:
          break;
    }
}

static void config_save_finished(bool success)
{
    isStoring = false;
    if (success)
    {
        NRF_LOG_INFO("New data record written to flash");
    }
    if (updatedWhileStoring) {
        updatedWhileStoring = false;
        save_rileylink_config();
    }
}


void rileylink_config_init(rileylink_config_ready_callback_t ready_callback) 
{
    m_ready_callback = ready_callback;
    err_code = fds_register(flash_callback);
    APP_ERROR_CHECK(err_code);
}