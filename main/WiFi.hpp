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
    WiFi(std::string_view ssid, std::string_view password);
    ~WiFi();

public:
    DELETE_COPY_MOVE(WiFi)

public:
    static constexpr uint32_t DEFAULT_TIMEOUT_MS = 120000;
    bool connect(uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);
    bool is_connected() const;
    bool connect_and_sync(uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);

private:
    void init();
    void disconnect();

private:
    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

private:
    const std::string_view _ssid;
    const std::string_view _password;
    bool _is_connected;
    bool _initialized;
    esp_netif_t* _netif;
};
