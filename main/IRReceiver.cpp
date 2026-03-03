#include "IRReceiver.hpp"

#include <memory>

#include "IRReceiverIter.hpp"
#include "esp_err.h"
#include "Exception.hpp"

static constexpr uint32_t RECEIVER_CARRIER_FREQ_HZ = 25000;

IRReceiver::IRReceiver(gpio_num_t gpio_num, const bool only_accpet_valid_ir) : 
    _channel(create_channel(gpio_num)),
    _iter(IRReceiverIter::create(_channel, only_accpet_valid_ir))
{
    _channel->enable();
    _iter->initiate_receive();
}


std::shared_ptr<RMTChannel> IRReceiver::create_channel(gpio_num_t gpio_num)
{   
    rmt_channel_handle_t handle = nullptr;
    
    const rmt_rx_channel_config_t rx_chan_config = {
        .gpio_num = gpio_num,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000 * 1000,
        .mem_block_symbols = 64,
        .intr_priority = 0,
        .flags = { .invert_in = false, .with_dma = false, .io_loop_back = false },
    };

    esp_err_t ret = rmt_new_rx_channel(&rx_chan_config, &handle);
    if (ret != ESP_OK) 
    {
        throw IRException(IRErrorCode::RX_CHANNEL_CREATE_FAILED);
    }

    return std::make_shared<RMTChannel>(handle, RECEIVER_CARRIER_FREQ_HZ);
}
