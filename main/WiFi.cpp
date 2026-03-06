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

WiFi::WiFi(std::string_view ssid, std::string_view password)
    : _ssid(ssid),
    _password(password),
    _is_connected(false),
    _initialized(false),
    _netif(nullptr)
{
    s_wifi_event_group = xEventGroupCreate();
    init();
}

WiFi::~WiFi()
{
    if (_initialized)
    {
        esp_err_t ret = esp_wifi_stop();
        if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_STARTED)
        {
            printf("[WiFi] WARNING: esp_wifi_stop failed: %d\n", ret);
        }

        ret = esp_wifi_deinit();
        if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_INIT)
        {
            printf("[WiFi] WARNING: esp_wifi_deinit failed: %d\n", ret);
        }

        if (_netif != nullptr)
        {
            esp_netif_destroy_default_wifi(_netif);
            _netif = nullptr;
        }

        ret = esp_event_loop_delete_default();
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
        {
            printf("[WiFi] WARNING: esp_event_loop_delete_default failed: %d\n", ret);
        }

        ret = esp_netif_deinit();
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
        {
            printf("[WiFi] WARNING: esp_netif_deinit failed: %d\n", ret);
        }

        _initialized = false;
    }

    if (s_wifi_event_group != nullptr)
    {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = nullptr;
    }
}

void WiFi::init()
{
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

    _netif = esp_netif_create_default_wifi_sta();

    const wifi_init_config_t wifi_conf = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&wifi_conf);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
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
}

bool WiFi::connect(uint32_t timeout_ms)
{
    printf("[WiFi] connect() started\n");

    bool retry_forever = (timeout_ms == 0);

    while (true)
    {
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT | FAILED_BIT);

        wifi_sta_config_t sta_conf = {};
        std::memcpy(sta_conf.ssid, _ssid.data(), _ssid.size());
        sta_conf.ssid[_ssid.size()] = '\0';
        std::memcpy(sta_conf.password, _password.data(), _password.size());
        sta_conf.password[_password.size()] = '\0';
        sta_conf.scan_method = WIFI_FAST_SCAN;

        wifi_config_t wifi_conf = {.sta = sta_conf};

        printf("[WiFi] Setting WiFi config...\n");
        esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_conf);
        if (ret != ESP_OK)
        {
            printf("[WiFi] ERROR: esp_wifi_set_config failed: %d\n", ret);
            if (!retry_forever)
            {
                _is_connected = false;
                return false;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        printf("[WiFi] Starting WiFi...\n");
        ret = esp_wifi_start();
        if (ret != ESP_OK && ret != ESP_ERR_WIFI_STATE)
        {
            printf("[WiFi] ERROR: esp_wifi_start failed: %d\n", ret);
            if (!retry_forever)
            {
                _is_connected = false;
                return false;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        printf("[WiFi] WiFi started\n");

        if (_is_connected)
        {
            printf("[WiFi] Was already connected, disconnecting first...\n");
            esp_wifi_disconnect();
        }

        printf("[WiFi] Connecting to AP...\n");
        ret = esp_wifi_connect();
        if (ret != ESP_OK)
        {
            printf("[WiFi] ERROR: esp_wifi_connect failed: %d\n", ret);
            if (!retry_forever)
            {
                _is_connected = false;
                return false;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        printf("[WiFi] WiFi connect() called, waiting for IP...\n");

        int64_t start_time = esp_timer_get_time() / 1000;
        bool timeout_not_reached = true;
        while (timeout_not_reached)
        {
            EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | FAILED_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(100));
            if (bits & CONNECTED_BIT)
            {
                printf("[WiFi] SUCCESS: Got IP address!\n");
                _is_connected = true;
                return true;
            }
            if (bits & FAILED_BIT)
            {
                printf("[WiFi] ERROR: Connection failed (disconnected)\n");
                break;
            }

            if (!retry_forever)
            {
                int64_t elapsed = (esp_timer_get_time() / 1000) - start_time;
                timeout_not_reached = elapsed < static_cast<int64_t>(timeout_ms);
            }
        }

        if (!retry_forever)
        {
            printf("[WiFi] ERROR: Timeout waiting for IP\n");
            _is_connected = false;
            return false;
        }

        printf("[WiFi] Retrying connection...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void WiFi::disconnect()
{
    printf("[WiFi] disconnect() called\n");
    esp_wifi_stop();
    _is_connected = false;
}

bool WiFi::is_connected() const
{
    esp_netif_t* netif = esp_netif_get_default_netif();
    if (netif == nullptr)
    {
        return false;
    }

    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK)
    {
        return false;
    }

    return ip_info.ip.addr != 0;
}

bool WiFi::connect_and_sync(uint32_t timeout_ms)
{
    if (!connect(timeout_ms))
    {
        return false;
    }

    return Utils::sync_time(*this);
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
