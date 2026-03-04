#include "WiFi.hpp"

#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

WiFi::WiFi(std::string_view ssid, std::string_view password)
    : _ssid(ssid),
    _password(password),
    _is_connected(false),
    _initialized(false)
{
    init();
    connect();
}

WiFi::~WiFi()
{
    disconnect();
}

void WiFi::init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK)
    {
        throw ret;
    }

    ret = esp_netif_init();
    if (ret != ESP_OK)
    {
        throw ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        throw ret;
    }

    esp_netif_create_default_wifi_sta();

    const wifi_init_config_t wifi_conf = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&wifi_conf);
    if (ret != ESP_OK)
    {
        throw ret;
    }

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK)
    {
        throw ret;
    }

    _initialized = true;
}

void WiFi::connect()
{
    if (!_initialized)
    {
        init();
    }

    wifi_sta_config_t sta_conf = {};
    std::memcpy(sta_conf.ssid, _ssid.data(), _ssid.size());
    sta_conf.ssid[_ssid.size()] = '\0';
    std::memcpy(sta_conf.password, _password.data(), _password.size());
    sta_conf.password[_password.size()] = '\0';
    sta_conf.scan_method = WIFI_ALL_CHANNEL_SCAN;
    
    wifi_config_t wifi_conf = {.sta = sta_conf};

    esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_conf);
    if (ret != ESP_OK)
    {
        throw ret;
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK)
    {
        throw ret;
    }

    ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        throw ret;
    }

    wifi_ap_record_t info;
    while (esp_wifi_sta_get_ap_info(&info) != ESP_OK)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_default_netif();
    while (netif == nullptr || esp_netif_get_ip_info(netif, &ip_info) != ESP_OK || ip_info.ip.addr == 0)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        netif = esp_netif_get_default_netif();
    }

    _is_connected = true;
}

void WiFi::disconnect()
{
    if (_initialized)
    {
        esp_wifi_stop();
        esp_wifi_deinit();
        _initialized = false;
    }
}

bool WiFi::is_connected() const
{
    return _is_connected;
}

void WiFi::reconnect()
{
    if (!_is_connected)
    {
        connect();
    }
}
