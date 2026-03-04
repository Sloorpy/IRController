#pragma once

#include <string_view>
#include <memory>

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
    bool is_connected() const;
    void reconnect();

private:
    void init();
    void connect();
    void disconnect();

private:
    const std::string_view _ssid;
    const std::string_view _password;
    bool _is_connected;
    bool _initialized;
};
