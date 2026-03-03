#pragma once

#include <memory>
#include <vector>

#include "driver/rmt_rx.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "IRCommand.hpp"
#include "Macros.hpp"

class RMTChannel;

class IRReceiverIter
{
friend class IRReceiver;

private:
    explicit IRReceiverIter(std::weak_ptr<RMTChannel> base, const bool only_accpet_valid_ir);

public:
    ~IRReceiverIter();

public:
    DELETE_COPY_MOVE(IRReceiverIter);

public:
    std::shared_ptr<RMTChannel> get_base() const;
    IRCommand receive();

private:    
    static std::unique_ptr<IRReceiverIter> create(std::weak_ptr<RMTChannel> base, const bool only_accpet_valid_ir);

private:
    void initialize_callback();
    void initiate_receive();
    
private:
    static bool receive_callback(rmt_channel_handle_t rx_chan, const rmt_rx_done_event_data_t *edata, void *user_ctx);

private:
    std::weak_ptr<RMTChannel> _base;
    std::vector<rmt_symbol_word_t> _symbols_buffer;
    QueueHandle_t _queue;
    const bool _only_accpet_valid_ir;
};
