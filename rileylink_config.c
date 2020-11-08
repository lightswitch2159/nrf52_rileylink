
#include "fds.h"
#include "nrf_log.h"
#include "rileylink_service.h"
#include "rileylink_config.h"

#define RL_CONFIG_FILE_ID 0x1001
#define RL_CONFIG_KEY 0x1000


__ALIGN(4) rileylink_config_t rileylink_config;

__ALIGN(4) rileylink_config_t rileylink_config_buffer;

volatile static bool m_isBusy = false;
volatile static bool m_pendingSave = false;
volatile static bool m_initialized = false;

rileylink_config_ready_callback_t m_ready_callback;

void rileylink_config_save()
{
    if (m_isBusy) {
      m_pendingSave = true;
      return;
    }

    m_isBusy = true;

    memcpy(&rileylink_config_buffer, &rileylink_config, sizeof(rileylink_config));

    fds_record_desc_t  record_desc;
    fds_record_t       record;

    memset(&record, 0, sizeof(record));

    record.file_id           = RL_CONFIG_FILE_ID;
    record.key               = RL_CONFIG_KEY;
    record.data.length_words = sizeof(rileylink_config) / 4;
    record.data.p_data       = (uint8_t *) &rileylink_config_buffer;

    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

    ret_code_t err_code = fds_record_find(RL_CONFIG_FILE_ID, RL_CONFIG_KEY , &desc, &tok);

    if (err_code == FDS_ERR_NOT_FOUND)
    {
        NRF_LOG_INFO("Writing %d bytes to fds", record.data.length_words);
        err_code = fds_record_write(&record_desc, &record);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("fds_record_write() failed for RL Config: %s", nrf_strerror_get(err_code))
            m_isBusy = false;
        }
    }
    else if (err_code == NRF_SUCCESS)
    {
        NRF_LOG_INFO("Updating %d bytes to fds", record.data.length_words);
        err_code = fds_record_update(&desc, &record);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("fds_record_update() failed for RL Config: %s", nrf_strerror_get(err_code))
            m_isBusy = false;
        }
    }
    else
    {
        NRF_LOG_ERROR("Error in fds_record_find: %s", nrf_strerror_get(err_code));
    }
}

static void init_default_config() {
    rileylink_config.version = 1;
    memcpy(rileylink_config.custom_name, DEFAULT_DEVICE_NAME, strlen(DEFAULT_DEVICE_NAME));
    rileylink_config.custom_name_len = strlen(DEFAULT_DEVICE_NAME);
}

static ret_code_t read_rileylink_config()
{
    ret_code_t         err_code;
    fds_record_desc_t  record_desc;
    fds_flash_record_t record;
    fds_find_token_t   ftok;

    memset(&ftok, 0, sizeof(ftok));

    err_code = fds_record_find(RL_CONFIG_FILE_ID, RL_CONFIG_KEY, &record_desc, &ftok);

    if (err_code == FDS_ERR_NOT_FOUND) {
        NRF_LOG_DEBUG("No config found; using default configuration.");
        init_default_config();
        return NRF_SUCCESS;
    }

    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("fds_record_find failed: %s", nrf_strerror_get(err_code));
        return err_code;
    }

    err_code = fds_record_open(&record_desc, &record);

    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("fds_record_open failed: %s", nrf_strerror_get(err_code));
        return err_code;
    }

    memcpy(&rileylink_config, record.p_data, sizeof(rileylink_config));

    NRF_LOG_DEBUG("config read: name = %s", rileylink_config.custom_name);

    err_code = fds_record_close(&record_desc); 
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("fds_record_close failed: %s", nrf_strerror_get(err_code));
        return err_code;
    }

    return NRF_SUCCESS;
}

static void fds_is_ready() {
    if (!m_initialized) {
        ret_code_t err_code;
        err_code = read_rileylink_config();
        m_initialized = true;
        if (m_ready_callback != NULL) {
            m_ready_callback(err_code == NRF_SUCCESS);
        }
    }
}

static void config_save_finished(bool success)
{
    if (success)
    {
        NRF_LOG_INFO("New data record written to flash");
    }
    m_isBusy = false;
    if (m_pendingSave) {
        m_pendingSave = false;
        rileylink_config_save();
    }
}

static void flash_callback(fds_evt_t const * p_evt)
{
    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if(p_evt->result == NRF_SUCCESS)
            {
                NRF_LOG_INFO("FDS initialized");
                fds_is_ready();
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
            if(p_evt->write.record_key == RL_CONFIG_KEY && p_evt->write.file_id == RL_CONFIG_FILE_ID)
            {
                NRF_LOG_INFO("FDS_EVT_WRITE config");
                config_save_finished(p_evt->result == NRF_SUCCESS);
            }
            break;
        case FDS_EVT_UPDATE:
            if(p_evt->write.record_key == RL_CONFIG_KEY && p_evt->write.file_id == RL_CONFIG_FILE_ID)
            {
                NRF_LOG_INFO("FDS_EVT_UPDATE config");
                config_save_finished(p_evt->result == NRF_SUCCESS);
            }
            break;
        case FDS_EVT_DEL_RECORD:
        case FDS_EVT_DEL_FILE:
        case FDS_EVT_GC:
          break;
    }
}

void rileylink_config_init(rileylink_config_ready_callback_t ready_callback) 
{
    ret_code_t err_code;
    
    m_ready_callback = ready_callback;
    err_code = fds_register(flash_callback);
    APP_ERROR_CHECK(err_code);

    err_code = fds_init();
    APP_ERROR_CHECK(err_code);
}