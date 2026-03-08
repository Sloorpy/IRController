#include "Storage.hpp"
#include "json.hpp"

#include <cstring>
#include <esp_log.h>

using nlohmann::json;

static constexpr char TAG[] = "Storage";

Storage::Storage()
    : _handle(0),
    _initialized(false)
{
}

Storage::~Storage()
{
    if (_initialized)
    {
        nvs_close(_handle);
    }
}

bool Storage::init()
{
    if (_initialized)
    {
        return true;
    }

    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = open_nvs(NVS_READWRITE, _handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return false;
    }

    _initialized = true;
    return true;
}

esp_err_t Storage::open_nvs(nvs_open_mode_t mode, nvs_handle_t& handle)
{
    return nvs_open(NVS_NAMESPACE, mode, &handle);
}

std::string Storage::get_string(const char* key, const char* default_val)
{
    if (!_initialized)
    {
        return default_val ? std::string(default_val) : std::string();
    }

    size_t required_size = 0;
    esp_err_t err = nvs_get_str(_handle, key, nullptr, &required_size);
    
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        return default_val ? std::string(default_val) : std::string();
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_get_str (%s) failed: %s", key, esp_err_to_name(err));
        return default_val ? std::string(default_val) : std::string();
    }

    if (required_size == 0)
    {
        return default_val ? std::string(default_val) : std::string();
    }

    std::string value(required_size, '\0');
    err = nvs_get_str(_handle, key, value.data(), &required_size);
    
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_get_str (%s) failed: %s", key, esp_err_to_name(err));
        return default_val ? std::string(default_val) : std::string();
    }

    value.resize(required_size - 1);
    return value;
}

void Storage::set_string(const char* key, const std::string& value)
{
    if (!_initialized)
    {
        return;
    }

    esp_err_t err = nvs_set_str(_handle, key, value.c_str());
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_set_str (%s) failed: %s", key, esp_err_to_name(err));
        return;
    }

    err = nvs_commit(_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    }
}

bool Storage::get_wifi_creds(std::string& ssid, std::string& password)
{
    ssid = get_string(KEY_WIFI_SSID, "");
    password = get_string(KEY_WIFI_PASS, "");
    return !ssid.empty();
}

void Storage::set_wifi_creds(const std::string& ssid, const std::string& password)
{
    set_string(KEY_WIFI_SSID, ssid);
    set_string(KEY_WIFI_PASS, password);
}

void Storage::clear_wifi_creds()
{
    if (!_initialized)
    {
        return;
    }

    nvs_erase_key(_handle, KEY_WIFI_SSID);
    nvs_erase_key(_handle, KEY_WIFI_PASS);
    nvs_commit(_handle);
}

std::vector<std::string> Storage::get_cities()
{
    std::string cities_json = get_string(KEY_CITIES, "");
    std::vector<std::string> cities;

    if (cities_json.empty())
    {
        return cities;
    }

    try
    {
        json j = json::parse(cities_json);
        if (j.is_array())
        {
            for (const auto& city : j)
            {
                if (city.is_string())
                {
                    cities.push_back(city.get<std::string>());
                }
            }
        }
    }
    catch (...)
    {
        ESP_LOGE(TAG, "Failed to parse cities JSON");
    }

    return cities;
}

void Storage::set_cities(const std::vector<std::string>& cities)
{
    json j = json::array();
    for (const auto& city : cities)
    {
        j.push_back(city);
    }

    set_string(KEY_CITIES, j.dump());
}

uint32_t Storage::get_alert_duration()
{
    if (!_initialized)
    {
        return 120;
    }

    uint32_t value = 120;
    esp_err_t err = nvs_get_u32(_handle, KEY_ALERT_DURATION, &value);
    
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        return 120;
    }

    return value;
}

void Storage::set_alert_duration(uint32_t seconds)
{
    if (!_initialized)
    {
        return;
    }

    nvs_set_u32(_handle, KEY_ALERT_DURATION, seconds);
    nvs_commit(_handle);
}

std::string Storage::get_led_sequence_warning()
{
    return get_string(KEY_LED_SEQUENCE_WARN, "");
}

void Storage::set_led_sequence_warning(const std::string& json)
{
    set_string(KEY_LED_SEQUENCE_WARN, json);
}

std::string Storage::get_led_sequence_rocket()
{
    return get_string(KEY_LED_SEQUENCE_ROCKET, "");
}

void Storage::set_led_sequence_rocket(const std::string& json)
{
    set_string(KEY_LED_SEQUENCE_ROCKET, json);
}

bool Storage::has_valid_config()
{
    std::string ssid = get_string(KEY_WIFI_SSID, "");
    return !ssid.empty();
}
