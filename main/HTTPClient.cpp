#include "HTTPClient.hpp"
#include "esp_log.h"

#include <cstring>

static constexpr size_t HTTP_BUFFER_SIZE = 4096 * 2;

HTTPClient::HTTPClient()
    : _buffer(HTTP_BUFFER_SIZE, '\0'),
    _response_received(false)
{
}

HTTPClient::~HTTPClient()
{
}

esp_err_t HTTPClient::_event_handler(esp_http_client_event_t* evt)
{
    if (evt->event_id == HTTP_EVENT_ON_DATA)
    {
        HTTPClient* client = static_cast<HTTPClient*>(evt->user_data);
        size_t len = strlen(client->_buffer.data());
        size_t copy_len = (evt->data_len < HTTP_BUFFER_SIZE - len - 1) ? evt->data_len : HTTP_BUFFER_SIZE - len - 1;
        if (copy_len > 0)
        {
            std::memcpy(client->_buffer.data() + len, evt->data, copy_len);
            client->_buffer[len + copy_len] = '\0';
        }
    }
    else if (evt->event_id == HTTP_EVENT_ON_FINISH)
    {
        HTTPClient* client = static_cast<HTTPClient*>(evt->user_data);
        client->_response_received = true;
    }
    return ESP_OK;
}

std::string HTTPClient::get(std::string_view url)
{
    _buffer.resize(0);
    _buffer.assign(HTTP_BUFFER_SIZE, '\0');
    _response_received = false;

    esp_http_client_config_t config = {
        .url = url.data(),
        .event_handler = _event_handler,
        .user_data = this,
        .skip_cert_common_name_check = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr)
    {
        throw ESP_FAIL;
    }

    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    if (err != ESP_OK)
    {
        throw err;
    }

    return std::string(_buffer.data());
}

json HTTPClient::get_json(std::string_view url)
{
    std::string get_result = get(url);

    static constexpr size_t MINIMUM_RESULT_LEN = 6;
    if (get_result.length() < MINIMUM_RESULT_LEN)
    {
        return json({});
    }

    return json::parse(get_result);
}

json HTTPClient::get_json_first(std::string_view url)
{
    std::string raw = get(url);

    static constexpr size_t MINIMUM_RESULT_LEN = 6;
    if (raw.length() < MINIMUM_RESULT_LEN)
    {
        return json({});
    }

    size_t start = raw.find('{');
    if (start == std::string::npos)
    {
        start = raw.find('[');
    }
    if (start == std::string::npos)
    {
        return json({});
    }

    int depth = 0;
    size_t end = start;
    for (size_t i = start; i < raw.size(); i++)
    {
        if (raw[i] == '{' || raw[i] == '[')
        {
            depth++;
        }
        else if (raw[i] == '}' || raw[i] == ']')
        {
            depth--;
        }

        if (depth == 0)
        {
            end = i + 1;
            break;
        }
    }

    return json::parse(raw.substr(start, end - start));
}