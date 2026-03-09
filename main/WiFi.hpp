#pragma once

#include <string_view>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs_flash.h>

#include "Macros.hpp"

class WiFi
{
public:
    explicit WiFi();
    ~WiFi();

    DELETE_COPY_MOVE(WiFi)

    void init();

    void enable_sta(std::string_view ssid, std::string_view password);
    void disable_sta();
    bool is_sta_connected() const;
    bool connect_sta(uint32_t timeout_ms);
    bool connect_sta_and_sync(uint32_t timeout_ms);

    void enable_ap(std::string_view ssid);
    void disable_ap();
    bool is_ap_started() const;

    void disable();

private:
    void create_ap_netif();
    void create_sta_netif();

    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    bool _initialized = false;
    bool _sta_enabled = false;
    bool _ap_enabled = false;
    bool _sta_connected = false;

    std::string_view _sta_ssid;
    std::string_view _sta_password;

    esp_netif_t* _sta_netif = nullptr;
    esp_netif_t* _ap_netif = nullptr;

    static bool s_sta_connected;
    static bool s_sta_connect_failed;
};
