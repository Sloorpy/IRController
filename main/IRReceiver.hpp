#pragma once

#include <memory>

#include "driver/gpio.h"

#include "RMTChannel.hpp"
#include "IRReceiverIter.hpp"

class IRReceiver
{
public:
    explicit IRReceiver(gpio_num_t gpio_num);
    ~IRReceiver() = default;

public:
    IRReceiverIter& get_receiver() { return *_iter; };

private:
    static std::shared_ptr<RMTChannel> create_channel(gpio_num_t gpio_num);

private:
    std::shared_ptr<RMTChannel> _channel;
    std::unique_ptr<IRReceiverIter> _iter;
};
