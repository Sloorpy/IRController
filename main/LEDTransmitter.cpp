#include "LEDTransmitter.hpp"

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

