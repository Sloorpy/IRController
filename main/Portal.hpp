#pragma once

#include <string>
#include <memory>
#include <functional>

#include <esp_err.h>
#include <esp_netif_types.h>
#include <esp_http_server.h>

#include "json.hpp"
#include "Macros.hpp"

using nlohmann::json;

class Storage;

class Portal
{
public:
    explicit Portal(Storage& storage);
    ~Portal();

    DELETE_COPY_MOVE(Portal)

    bool start();
    void stop();

    using WiFiConnectCallback = std::function<void(const std::string& ssid, const std::string& password)>;
    using RestartCallback = std::function<void()>;
    using OTACallback = std::function<void()>;
    using TestLightsCallback = std::function<void()>;

    void set_wifi_connect_callback(WiFiConnectCallback cb);
    void set_restart_callback(RestartCallback cb);
    void set_ota_callback(OTACallback cb);
    void set_test_lights_callback(TestLightsCallback cb);

    bool is_sta_connected() const { return _sta_connected; }
    void set_sta_connected(bool connected) { _sta_connected = connected; }
    std::string get_sta_ssid() const { return _sta_ssid; }
    void set_sta_ssid(const std::string& ssid) { _sta_ssid = ssid; }

private:
    static esp_err_t root_handler(httpd_req_t* req);
    static esp_err_t api_wifi_connect_handler(httpd_req_t* req);
    static esp_err_t api_wifi_status_handler(httpd_req_t* req);
    static esp_err_t api_led_sequence_handler(httpd_req_t* req);
    static esp_err_t api_led_duration_handler(httpd_req_t* req);
    static esp_err_t api_ir_record_handler(httpd_req_t* req);
    static esp_err_t api_system_restart_handler(httpd_req_t* req);
    static esp_err_t api_system_update_handler(httpd_req_t* req);
    static esp_err_t api_system_test_lights_handler(httpd_req_t* req);
    static esp_err_t api_system_logs_handler(httpd_req_t* req);
    static esp_err_t api_cities_handler(httpd_req_t* req);
    static esp_err_t captive_check_handler(httpd_req_t* req);
    static esp_err_t catchall_handler(httpd_req_t* req);

    void add_log(const char* fmt, ...);

    Storage& _storage;
    httpd_handle_t _server = nullptr;
    bool _running = false;
    bool _sta_connected = false;
    std::string _sta_ssid;

    WiFiConnectCallback _wifi_connect_cb;
    RestartCallback _restart_cb;
    OTACallback _ota_cb;
    TestLightsCallback _test_lights_cb;

    std::string _log_buffer;
    static Portal* s_portal_instance;
};
