#include "WiFi.hpp"

#include <cstring>
#include <stdio.h>

#include "Exception.hpp"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "Utils.hpp"

static constexpr int CONNECTED_BIT = BIT0;
static constexpr int FAILED_BIT = BIT1;
static EventGroupHandle_t s_wifi_event_group = nullptr;

bool WiFi::s_sta_connected = false;
bool WiFi::s_sta_connect_failed = false;

WiFi::WiFi()
    : _initialized(false),
    _sta_enabled(false),
    _ap_enabled(false),
    _sta_connected(false)
{
}

WiFi::~WiFi()
{
    disable();
}

void WiFi::init()
{
    if (_initialized)
    {
        return;
    }

    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = esp_netif_init();
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        throw EspException(ret);
    }

    s_wifi_event_group = xEventGroupCreate();

    const wifi_init_config_t wifi_conf = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&wifi_conf);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = esp_wifi_set_mode(WIFI_MODE_NULL);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFi::event_handler, nullptr);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFi::event_handler, nullptr);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    _initialized = true;
    printf("[WiFi] Initialized\n");
}

void WiFi::enable_ap(std::string_view ssid)
{
    if (!_initialized)
    {
        init();
    }

    if (_ap_enabled)
    {
        printf("[WiFi] AP already enabled\n");
        return;
    }

    create_ap_netif();

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, ssid.data(), sizeof(wifi_config.ap.ssid) - 1);
    wifi_config.ap.ssid_len = ssid.length();
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    wifi_mode_t current_mode = WIFI_MODE_NULL;
    esp_wifi_get_mode(&current_mode);

    wifi_mode_t new_mode = static_cast<wifi_mode_t>(current_mode | WIFI_MODE_AP);
    esp_err_t ret = esp_wifi_set_mode(new_mode);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    _ap_enabled = true;
    printf("[WiFi] AP enabled: %.*s\n", ssid.length(), ssid.data());
}

void WiFi::disable_ap()
{
    if (!_ap_enabled)
    {
        return;
    }

    esp_wifi_set_mode(WIFI_MODE_STA);
    _ap_enabled = false;
    printf("[WiFi] AP disabled\n");
}

bool WiFi::is_ap_started() const
{
    return _ap_enabled;
}

void WiFi::enable_sta(std::string_view ssid, std::string_view password)
{
    if (!_initialized)
    {
        init();
    }

    _sta_ssid = ssid;
    _sta_password = password;
    _sta_enabled = true;
    printf("[WiFi] STA enabled: %.*s\n", ssid.length(), ssid.data());
}

void WiFi::disable_sta()
{
    if (!_sta_enabled)
    {
        return;
    }

    esp_wifi_disconnect();
    _sta_enabled = false;
    _sta_connected = false;
    printf("[WiFi] STA disabled\n");
}

bool WiFi::is_sta_connected() const
{
    return _sta_connected;
}

bool WiFi::connect_sta(uint32_t timeout_ms)
{
    if (!_sta_enabled || _sta_ssid.empty())
    {
        printf("[WiFi] STA not enabled\n");
        return false;
    }

    wifi_sta_config_t sta_conf = {};
    std::memcpy(sta_conf.ssid, _sta_ssid.data(), _sta_ssid.size());
    sta_conf.ssid[_sta_ssid.size()] = '\0';
    std::memcpy(sta_conf.password, _sta_password.data(), _sta_password.size());
    sta_conf.password[_sta_password.size()] = '\0';
    sta_conf.scan_method = WIFI_FAST_SCAN;

    wifi_config_t wifi_conf = {.sta = sta_conf};

    wifi_mode_t current_mode = WIFI_MODE_NULL;
    esp_wifi_get_mode(&current_mode);

    wifi_mode_t new_mode = static_cast<wifi_mode_t>(current_mode | WIFI_MODE_STA);
    esp_err_t ret = esp_wifi_set_mode(new_mode);
    if (ret != ESP_OK)
    {
        printf("[WiFi] Failed to set mode: %d\n", ret);
        return false;
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_conf);
    if (ret != ESP_OK)
    {
        printf("[WiFi] Failed to set config: %d\n", ret);
        return false;
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK && ret != ESP_ERR_WIFI_STATE)
    {
        printf("[WiFi] Failed to start WiFi: %d\n", ret);
        return false;
    }

    s_sta_connected = false;
    s_sta_connect_failed = false;

    ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        printf("[WiFi] Failed to connect: %d\n", ret);
        return false;
    }

    int64_t start_time = esp_timer_get_time() / 1000;
    while (true)
    {
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | FAILED_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(100));

        if (bits & CONNECTED_BIT)
        {
            _sta_connected = true;
            s_sta_connected = true;
            printf("[WiFi] Connected!\n");
            return true;
        }

        if (bits & FAILED_BIT)
        {
            printf("[WiFi] Connection failed\n");
            _sta_connected = false;
            s_sta_connect_failed = true;
            return false;
        }

        if (timeout_ms > 0)
        {
            int64_t elapsed = (esp_timer_get_time() / 1000) - start_time;
            if (elapsed >= static_cast<int64_t>(timeout_ms))
            {
                printf("[WiFi] Connection timeout\n");
                _sta_connected = false;
                return false;
            }
        }
    }
}

bool WiFi::connect_sta_and_sync(uint32_t timeout_ms)
{
    if (!connect_sta(timeout_ms))
    {
        return false;
    }

    return Utils::sync_time(*this);
}

void WiFi::disable()
{
    if (!_initialized)
    {
        return;
    }

    if (_sta_enabled)
    {
        esp_wifi_disconnect();
        _sta_enabled = false;
    }

    if (_ap_enabled)
    {
        esp_wifi_stop();
        _ap_enabled = false;
    }

    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(_sta_netif);
    esp_netif_destroy_default_wifi(_ap_netif);
    esp_event_loop_delete_default();
    esp_netif_deinit();

    if (s_wifi_event_group != nullptr)
    {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = nullptr;
    }

    _initialized = false;
    printf("[WiFi] Disabled\n");
}

void WiFi::create_ap_netif()
{
    if (_ap_netif == nullptr)
    {
        _ap_netif = esp_netif_create_default_wifi_ap();
    }
}

void WiFi::create_sta_netif()
{
    if (_sta_netif == nullptr)
    {
        _sta_netif = esp_netif_create_default_wifi_sta();
    }
}

void WiFi::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                printf("[WiFi] Event: WIFI_EVENT_STA_START\n");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                printf("[WiFi] Event: WIFI_EVENT_STA_CONNECTED\n");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                printf("[WiFi] Event: WIFI_EVENT_STA_DISCONNECTED\n");
                xEventGroupSetBits(s_wifi_event_group, FAILED_BIT);
                break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
            case IP_EVENT_STA_GOT_IP:
                printf("[WiFi] Event: IP_EVENT_STA_GOT_IP\n");
                xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
                break;
            case IP_EVENT_STA_LOST_IP:
                printf("[WiFi] Event: IP_EVENT_STA_LOST_IP\n");
                break;
        }
    }
}
