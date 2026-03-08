#pragma once

#include <string>
#include <vector>

#include <nvs_flash.h>
#include <nvs.h>
#include <esp_err.h>

#include "Macros.hpp"

static constexpr const char* NVS_NAMESPACE = "alertlight";
static constexpr const char* KEY_WIFI_SSID = "wifi_ssid";
static constexpr const char* KEY_WIFI_PASS = "wifi_pass";
static constexpr const char* KEY_CITIES = "cities";
static constexpr const char* KEY_ALERT_DURATION = "alert_dur";
static constexpr const char* KEY_LED_SEQUENCE_WARN = "led_warn";
static constexpr const char* KEY_LED_SEQUENCE_ROCKET = "led_rocket";

class Storage
{
public:
    explicit Storage();
    ~Storage();

    DELETE_COPY_MOVE(Storage)

    bool init();

    bool get_wifi_creds(std::string& ssid, std::string& password);
    void set_wifi_creds(const std::string& ssid, const std::string& password);
    void clear_wifi_creds();

    std::vector<std::string> get_cities();
    void set_cities(const std::vector<std::string>& cities);

    uint32_t get_alert_duration();
    void set_alert_duration(uint32_t seconds);

    std::string get_led_sequence_warning();
    void set_led_sequence_warning(const std::string& json);

    std::string get_led_sequence_rocket();
    void set_led_sequence_rocket(const std::string& json);

    bool has_valid_config();

private:
    esp_err_t open_nvs(nvs_open_mode_t mode, nvs_handle_t& handle);
    std::string get_string(const char* key, const char* default_val = "");
    void set_string(const char* key, const std::string& value);

    nvs_handle_t _handle;
    bool _initialized;
};
