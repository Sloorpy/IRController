#include "IRTransmitter.hpp"

#include <memory>
#include <vector>

#include "RMTChannel.hpp"
#include "driver/rmt_encoder.h"
#include "esp_log.h"
#include "Exception.hpp"
#include "IRCommand.hpp"
#include "driver/rmt_rx.h"
#include "driver/rmt_types.h"
#include "IRCommand.hpp"

static constexpr size_t DEFUALT_TX_QUEUE_DEPTH = 4;
static constexpr uint32_t RESOLUTION_HZ = 1000 * 1000;
static constexpr size_t MEM_BLOCK_SIZE = 64;
static constexpr float CARRIER_DUTY_CYCLE = 0.33f;
static constexpr bool CARRIER_POLARITY_ACTIVE_LOW = false;
static constexpr uint32_t TRANSMITTER_CARRIER_FREQ_HZ = 38222;

IRTransmitter::IRTransmitter(gpio_num_t gpio_num) : 
    _channel(create_channel(gpio_num))
{
    rmt_carrier_config_t carrier_cfg = {
        .frequency_hz = TRANSMITTER_CARRIER_FREQ_HZ,
        .duty_cycle = CARRIER_DUTY_CYCLE,
        .flags = { .polarity_active_low = CARRIER_POLARITY_ACTIVE_LOW },
    };
    rmt_apply_carrier(_channel->get_channel(), &carrier_cfg);

    _channel->enable();
}


std::shared_ptr<RMTChannel> IRTransmitter::create_channel(gpio_num_t gpio_num)
{
    rmt_channel_handle_t handle = nullptr;
    const rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = gpio_num,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RESOLUTION_HZ,
        .mem_block_symbols = MEM_BLOCK_SIZE,
        .trans_queue_depth = DEFUALT_TX_QUEUE_DEPTH,
        .flags = { .invert_out = false, .with_dma = false, .io_loop_back = false },
    };

    esp_err_t ret = rmt_new_tx_channel(&tx_chan_config, &handle);
    if (ret != ESP_OK) 
    {
        throw IRException(IRErrorCode::TX_CHANNEL_CREATE_FAILED);
    }

    return std::make_shared<RMTChannel>(handle, TRANSMITTER_CARRIER_FREQ_HZ);
}

void IRTransmitter::send(const IRCommand& cmd)
{
    auto timings = NECProtocol::encode(cmd.address, cmd.command);
    send_raw(timings);
}

void IRTransmitter::send_raw(const std::vector<uint16_t>& timings)
{
    std::vector<rmt_symbol_word_t> symbols;
    symbols.reserve(timings.size() / 2);

    static constexpr uint8_t SPACE_BIT = 0;
    static constexpr uint8_t BURST_BIT = 1;
    for (size_t i = 0; i < timings.size(); i+=2)
    {
        rmt_symbol_word_t symbol = {
            .duration0 = timings[i],
            .level0 = BURST_BIT,
            .duration1 = timings[i + 1],
            .level1 = SPACE_BIT,
        };
        symbols.push_back(symbol);
    }

    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };

    rmt_encoder_handle_t encoder = nullptr;
    rmt_copy_encoder_config_t encoder_config = {};
    esp_err_t ret = rmt_new_copy_encoder(&encoder_config, &encoder);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    ret = rmt_transmit(_channel->get_channel(), encoder, symbols.data(), symbols.size() * sizeof(rmt_symbol_word_t), &tx_config);
    if (ret != ESP_OK)
    {
        throw EspException(ret);
    }

    rmt_tx_wait_all_done(_channel->get_channel(), -1);
}
