#pragma once

#include <cstdint>

#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "esp_err.h"

#include "Macros.hpp"

class RMTChannel
{
public:
    explicit RMTChannel(rmt_channel_handle_t channel, uint32_t carrier_frequency_hz);
    virtual ~RMTChannel();

    DELETE_COPY_MOVE(RMTChannel)

    rmt_channel_handle_t get_channel() const { return _channel; }
    virtual void enable();

protected:
    rmt_channel_handle_t _channel = nullptr;
    uint32_t _carrier_frequency_hz = 0;
};
