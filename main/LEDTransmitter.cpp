#include "LEDTransmitter.hpp"
#include "esp_timer.h"
#include <inttypes.h>   // for PRId64 / PRIu64 if you want
#include <cstdio>  
#include <sys/unistd.h>

LEDTransmitter::LEDTransmitter(gpio_num_t gpio_num) :
    IRTransmitter(gpio_num)
{
}


void LEDTransmitter::send(const IRCommand& cmd)
{
    std::vector<uint16_t> timings = NECProtocol::encode_led(cmd.command);
    send_raw(timings);
}

void LEDTransmitter::send(const LEDProtocol& cmd)
{
    std::vector<uint16_t> timings = NECProtocol::encode_led(static_cast<uint8_t>(cmd));
    send_raw(timings);
}

void LEDTransmitter::send_multiple(const LEDProtocol& cmd, const uint16_t amount, const uint16_t delay)
{    
    std::vector<uint16_t> timings = NECProtocol::encode_led(static_cast<uint8_t>(cmd));
    
    for (uint16_t i = 0; i < amount; ++i)
    {
        static constexpr uint16_t MICRO_IN_MILLI = 1000;
        usleep(delay * MICRO_IN_MILLI);
        send_raw(timings);
    }
}

