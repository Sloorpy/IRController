#include "IRReceiverIter.hpp"

#include <memory>
#include <vector>

#include "RMTChannel.hpp"
#include "esp_log.h"
#include "Exception.hpp"
#include "IRCommand.hpp"
#include "driver/rmt_rx.h"
#include "driver/rmt_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static constexpr size_t RX_BUFFER_SIZE = 64;

IRReceiverIter::IRReceiverIter(std::weak_ptr<RMTChannel> base)
    : _base(base),
    _symbols_buffer(RX_BUFFER_SIZE),
    _queue(xQueueCreate(1, sizeof(IRCommand*)))
{
    if (_queue == nullptr)
    {
        throw Exception(ErrorCode::XQUEUE_CREATE_FAILED);
    }
}

IRReceiverIter::~IRReceiverIter()
{
    try
    {
        auto channel_owner = get_base();
        if (channel_owner)
        {
            rmt_channel_handle_t channel = channel_owner->get_channel();
            if (channel)
            {
                rmt_rx_event_callbacks_t cbs = {
                    .on_recv_done = nullptr,
                };
                rmt_rx_register_event_callbacks(channel, &cbs, nullptr);
            }
        }
    }
    catch (...)
    {
    }

    try
    {
        if (_queue != nullptr)
        {
            vQueueDelete(_queue);
        }
    }
    catch (...)
    {
    }
}

std::unique_ptr<IRReceiverIter> IRReceiverIter::create(std::weak_ptr<RMTChannel> base)
{
    std::unique_ptr<IRReceiverIter> iter(new IRReceiverIter(base));
    iter->initialize_callback();
    return iter;
}

void IRReceiverIter::initialize_callback()
{
    std::shared_ptr<RMTChannel> channel_owner = get_base();
    if (!channel_owner)
    {
        throw Exception(ErrorCode::RX_CHANNEL_CREATE_FAILED);
    }

    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = receive_callback,
    };

    esp_err_t ret = rmt_rx_register_event_callbacks(channel_owner->get_channel(), &cbs, this);
    if (ret != ESP_OK)
    {
        throw Exception(ErrorCode::RX_CALLBACK_REGISTER_FAILED);
    }

    receive_next();
}


std::shared_ptr<RMTChannel> IRReceiverIter::get_base() const
{
    return _base.lock();
}

void IRReceiverIter::receive_next()
{
    std::shared_ptr<RMTChannel> channel_owner = get_base();

    if (!channel_owner)
    {
        throw Exception(ErrorCode::RMT_CHANNEL_WAS_FREED);
    }

    rmt_receive_config_t receive_config = {
        .signal_range_min_ns = 1250,
        .signal_range_max_ns = 12000000,
    };

    esp_err_t ret = rmt_receive(channel_owner->get_channel(), _symbols_buffer.data(), _symbols_buffer.size() * sizeof(rmt_symbol_word_t), &receive_config);
    
    if (ret != ESP_OK)
    {
        throw Exception(ErrorCode::RMT_RECEIVE_FAILED);
    }
}

IRCommand IRReceiverIter::receive()
{
    IRCommand cmd;
    xQueueReceive(_queue, &cmd, portMAX_DELAY);
    return cmd;
}

bool IRReceiverIter::receive_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t* edata, void* user_data)
{   
    IRReceiverIter* iter = static_cast<IRReceiverIter*>(user_data);

    try 
    {
        std::vector<uint16_t> timings;
        timings.reserve(edata->num_symbols * 2);

        for (size_t i = 0; i < edata->num_symbols; i++)
        {
            uint32_t duration0 = edata->received_symbols[i].duration0;
            uint32_t duration1 = edata->received_symbols[i].duration1;

            if (duration0 > 0)
            {
                timings.push_back(static_cast<uint16_t>(duration0));
            }
            if (duration1 > 0)
            {
                timings.push_back(static_cast<uint16_t>(duration1));
            }
        }

        const IRCommand cmd = NECProtocol::decode(timings);

        xQueueGenericSend(iter->_queue, &cmd, portMAX_DELAY, queueSEND_TO_BACK);

        iter->receive_next();

        return true;
    }
    catch (Exception ex) 
    {       
        ESP_DRAM_LOGI("IR", "ERROR: crashed with code %d\n", static_cast<uint16_t>(ex.get()));
    }
    catch (...)
    {
        ESP_DRAM_LOGI("IR", "ERROR: crashed with unknown error");
    }
    
    iter->receive_next();

    return false;
}
