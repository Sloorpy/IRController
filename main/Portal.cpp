#include "Portal.hpp"
#include "Storage.hpp"
#include "PortalHTML.hpp"
#include "Cities.hpp"

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_http_server.h>
#include <sys/param.h>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using nlohmann::json;

static constexpr char TAG[] = "Portal";

Portal* Portal::s_portal_instance = nullptr;

static void send_json_response(httpd_req_t* req, bool success, const char* message)
{
    char buf[256];
    int len = snprintf(buf, sizeof(buf), 
        "{\"success\":%s,\"message\":\"%s\"}", 
        success ? "true" : "false", 
        message ? message : "");
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, buf, len);
}

static void send_json_response_with_data(httpd_req_t* req, const char* json)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, strlen(json));
}

Portal::Portal(Storage& storage)
    : _storage(storage),
    _running(false),
    _sta_connected(false)
{
    s_portal_instance = this;
    add_log("Portal created");
}

Portal::~Portal()
{
    stop();
    s_portal_instance = nullptr;
}

void Portal::add_log(const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    if (_log_buffer.size() > 8000)
    {
        _log_buffer = _log_buffer.substr(_log_buffer.size() - 4000);
    }
    _log_buffer += buf;
    _log_buffer += "\n";
}

bool Portal::start()
{
    if (_running)
    {
        return true;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port = 32768;
    config.max_open_sockets = 7;
    config.max_uri_handlers = 20;
    config.lru_purge_enable = true;
    config.uri_match_fn = httpd_uri_match_wildcard;

    add_log("Starting HTTP server...");

    if (httpd_start(&_server, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        add_log("Failed to start HTTP server");
        return false;
    }

    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &root_uri);

    httpd_uri_t api_wifi_connect = {
        .uri = "/api/wifi/connect",
        .method = HTTP_POST,
        .handler = api_wifi_connect_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_wifi_connect);

    httpd_uri_t api_wifi_status = {
        .uri = "/api/wifi/status",
        .method = HTTP_GET,
        .handler = api_wifi_status_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_wifi_status);

    httpd_uri_t api_led_sequence = {
        .uri = "/api/led/sequence",
        .method = HTTP_POST,
        .handler = api_led_sequence_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_led_sequence);

    httpd_uri_t api_led_duration = {
        .uri = "/api/led/duration",
        .method = HTTP_POST,
        .handler = api_led_duration_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_led_duration);

    httpd_uri_t api_ir_record = {
        .uri = "/api/ir/record",
        .method = HTTP_POST,
        .handler = api_ir_record_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_ir_record);

    httpd_uri_t api_system_restart = {
        .uri = "/api/system/restart",
        .method = HTTP_POST,
        .handler = api_system_restart_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_system_restart);

    httpd_uri_t api_system_update = {
        .uri = "/api/system/check_update",
        .method = HTTP_POST,
        .handler = api_system_update_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_system_update);

    httpd_uri_t api_system_test_lights = {
        .uri = "/api/system/test_lights",
        .method = HTTP_POST,
        .handler = api_system_test_lights_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_system_test_lights);

    httpd_uri_t api_system_logs = {
        .uri = "/api/system/logs",
        .method = HTTP_GET,
        .handler = api_system_logs_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_system_logs);

    httpd_uri_t api_cities = {
        .uri = "/api/cities",
        .method = HTTP_GET,
        .handler = api_cities_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &api_cities);

    httpd_uri_t captive_204 = {
        .uri = "/generate_204",
        .method = HTTP_GET,
        .handler = captive_check_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &captive_204);

    httpd_uri_t captive_hotspot = {
        .uri = "/hotspot-detect.html",
        .method = HTTP_GET,
        .handler = captive_check_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &captive_hotspot);

    httpd_uri_t catchall = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = catchall_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(_server, &catchall);

    _running = true;
    add_log("HTTP server started on port 80");
    return true;
}

void Portal::stop()
{
    if (_server)
    {
        httpd_stop(_server);
        _server = nullptr;
    }
    _running = false;
}

void Portal::set_wifi_connect_callback(WiFiConnectCallback cb)
{
    _wifi_connect_cb = cb;
}

void Portal::set_restart_callback(RestartCallback cb)
{
    _restart_cb = cb;
}

void Portal::set_ota_callback(OTACallback cb)
{
    _ota_cb = cb;
}

void Portal::set_test_lights_callback(TestLightsCallback cb)
{
    _test_lights_cb = cb;
}

esp_err_t Portal::root_handler(httpd_req_t* req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_send(req, PORTAL_HTML, strlen(PORTAL_HTML));
    return ESP_OK;
}

esp_err_t Portal::api_wifi_connect_handler(httpd_req_t* req)
{
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        send_json_response(req, false, "No data received");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    if (s_portal_instance)
    {
        s_portal_instance->add_log("WiFi connect request received");
    }

    try
    {
        json j = json::parse(buf);
        
        if (j.contains("ssid") && j["ssid"].is_string() &&
            j.contains("password") && j["password"].is_string())
        {
            std::string ssid = j["ssid"].get<std::string>();
            std::string password = j["password"].get<std::string>();

            if (s_portal_instance)
            {
                s_portal_instance->_sta_ssid = ssid;
                s_portal_instance->add_log("WiFi connection requested for: %s", ssid.c_str());
                
                if (s_portal_instance->_wifi_connect_cb)
                {
                    s_portal_instance->_wifi_connect_cb(ssid, password);
                }
            }

            send_json_response(req, true, "Connecting...");
        }
        else
        {
            send_json_response(req, false, "Invalid JSON format");
        }
    }
    catch (...)
    {
        send_json_response(req, false, "JSON parse error");
    }

    return ESP_OK;
}

esp_err_t Portal::api_wifi_status_handler(httpd_req_t* req)
{
    char buf[128];
    bool connected = false;
    std::string ssid;
    
    if (s_portal_instance)
    {
        connected = s_portal_instance->_sta_connected;
        ssid = s_portal_instance->_sta_ssid;
    }

    int len = snprintf(buf, sizeof(buf),
        "{\"connected\":%s,\"ssid\":\"%s\"}",
        connected ? "true" : "false",
        ssid.c_str());
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, buf, len);
    return ESP_OK;
}

esp_err_t Portal::api_led_sequence_handler(httpd_req_t* req)
{
    char buf[1024];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        send_json_response(req, false, "No data received");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    if (s_portal_instance)
    {
        s_portal_instance->add_log("LED sequence saved");
    }

    try
    {
        json j = json::parse(buf);
        
        if (j.contains("cities") && j["cities"].is_array())
        {
            std::vector<std::string> cities;
            for (const auto& city : j["cities"])
            {
                if (city.is_string())
                {
                    cities.push_back(city.get<std::string>());
                }
            }
            s_portal_instance->_storage.set_cities(cities);
        }
        
        if (j.contains("alertDuration") && j["alertDuration"].is_number())
        {
            uint32_t duration = j["alertDuration"].get<uint32_t>();
            s_portal_instance->_storage.set_alert_duration(duration);
        }
        
        if (j.contains("sequences"))
        {
            if (j["sequences"].contains("warning"))
            {
                s_portal_instance->_storage.set_led_sequence_warning(
                    j["sequences"]["warning"].dump());
            }
            if (j["sequences"].contains("rocket"))
            {
                s_portal_instance->_storage.set_led_sequence_rocket(
                    j["sequences"]["rocket"].dump());
            }
        }

        send_json_response(req, true, "Sequence saved");
    }
    catch (...)
    {
        send_json_response(req, false, "Error saving sequence");
    }

    return ESP_OK;
}

esp_err_t Portal::api_led_duration_handler(httpd_req_t* req)
{
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        send_json_response(req, false, "No data received");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    try
    {
        json j = json::parse(buf);
        if (j.contains("duration") && j["duration"].is_number())
        {
            uint32_t duration = j["duration"].get<uint32_t>();
            s_portal_instance->_storage.set_alert_duration(duration);
            send_json_response(req, true, "Duration saved");
        }
        else
        {
            send_json_response(req, false, "Invalid duration");
        }
    }
    catch (...)
    {
        send_json_response(req, false, "Error");
    }

    return ESP_OK;
}

esp_err_t Portal::api_ir_record_handler(httpd_req_t* req)
{
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        send_json_response(req, false, "No data received");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    if (s_portal_instance)
    {
        s_portal_instance->add_log("IR record request (stub)");
    }

    send_json_response(req, true, "calibrated:false");
    return ESP_OK;
}

esp_err_t Portal::api_system_restart_handler(httpd_req_t* req)
{
    send_json_response(req, true, "Restarting...");
    
    if (s_portal_instance && s_portal_instance->_restart_cb)
    {
        s_portal_instance->add_log("Restart requested");
        vTaskDelay(pdMS_TO_TICKS(500));
        s_portal_instance->_restart_cb();
    }
    
    return ESP_OK;
}

esp_err_t Portal::api_system_update_handler(httpd_req_t* req)
{
    send_json_response(req, true, "Checking for updates...");
    
    if (s_portal_instance && s_portal_instance->_ota_cb)
    {
        s_portal_instance->add_log("OTA check requested");
        s_portal_instance->_ota_cb();
    }
    
    return ESP_OK;
}

esp_err_t Portal::api_system_test_lights_handler(httpd_req_t* req)
{
    send_json_response(req, true, "Testing lights...");
    
    if (s_portal_instance && s_portal_instance->_test_lights_cb)
    {
        s_portal_instance->add_log("Test lights requested");
        s_portal_instance->_test_lights_cb();
    }
    
    return ESP_OK;
}

esp_err_t Portal::api_system_logs_handler(httpd_req_t* req)
{
    char buf[8192];
    std::string logs = "No logs";
    
    if (s_portal_instance)
    {
        logs = s_portal_instance->_log_buffer;
        if (logs.empty())
        {
            logs = "No logs yet";
        }
    }

    httpd_resp_set_type(req, "application/json");
    
    char response[8192];
    snprintf(response, sizeof(response), "{\"logs\":\"%s\"}", logs.c_str());
    
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

esp_err_t Portal::api_cities_handler(httpd_req_t* req)
{
    auto cities = s_portal_instance->_storage.get_cities();
    
    json j = json::array();
    for (size_t i = 0; i < get_initial_city_count(); ++i)
    {
        std::string city(get_initial_cities()[i].data(), get_initial_cities()[i].size());
        j.push_back(city);
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, j.dump().c_str(), j.dump().size());
    return ESP_OK;
}

esp_err_t Portal::captive_check_handler(httpd_req_t* req)
{
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}

esp_err_t Portal::catchall_handler(httpd_req_t* req)
{
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}
