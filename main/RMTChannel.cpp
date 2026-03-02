#include "RMTChannel.hpp"

#include "esp_err.h"
#include "Exception.hpp"

RMTChannel::RMTChannel(rmt_channel_handle_t channel, uint32_t carrier_frequency_hz) :
    _channel(channel),
    _carrier_frequency_hz(carrier_frequency_hz)
{
}

void RMTChannel::enable()
{
    esp_err_t ret = rmt_enable(_channel);
    if (ret != ESP_OK)
    {
        throw Exception(ErrorCode::ENABLE_CHANNEL_FAILED);
    }
}

RMTChannel::~RMTChannel()
{
    try
    {
        if (!_channel)
        {
            return;
        }

        rmt_disable(_channel);
        rmt_del_channel(_channel);
        _channel = nullptr;
    }
    catch (...)
    {
    }
}
