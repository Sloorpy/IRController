#pragma once

#include "Macros.hpp"
#include <string>
#include <string_view>
#include <vector>

#include <esp_err.h>
#include <esp_http_client.h>
#include "json.hpp"

using nlohmann::json;

class HTTPClient
{
public:
    HTTPClient();
    ~HTTPClient();

public:
    DELETE_COPY_MOVE(HTTPClient)

public:
    std::string get(std::string_view url);
    json get_json(std::string_view url);
    json get_json_first(std::string_view url);

private:
    static esp_err_t _event_handler(esp_http_client_event_t* evt);

    std::vector<char> _buffer;
    bool _response_received;
};
