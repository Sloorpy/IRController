#pragma once

#include <memory>
#include <vector>
#include "driver/gpio.h"

#include "RMTChannel.hpp"
#include "IRCommand.hpp"

class IRTransmitter
{
public:
    explicit IRTransmitter(gpio_num_t gpio_num);
    ~IRTransmitter() = default;

    virtual void send(const IRCommand& cmd);
    void send_raw(const std::vector<uint16_t>& timings);

private:
    static std::shared_ptr<RMTChannel> create_channel(gpio_num_t gpio_num);

private:
    std::shared_ptr<RMTChannel> _channel;
};
