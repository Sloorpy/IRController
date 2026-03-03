#pragma once

#include <memory>
#include <vector>
#include "IRCommand.hpp"
#include "driver/gpio.h"

#include "IRTransmitter.hpp"

enum class LEDProtocol : uint8_t
{
    POWER_ON = 0x3,
    POWER_OFF = 0x2,
    BRIGHT_UP = 0x0,
    BRIGHT_DOWN = 0x1,
    RED_LIGHT = 0x4,
    GREEN_LIGHT = 0x5,
    HARD_BLUE_LIGHT = 0x6,
    WHITE_LIGHT = 0x7,
    ORANGE_LIGHT = 0x8,
    CYAN_LIGHT = 0x9,
    PURPLE_LIGHT = 0xA,
    YELLOW_LIGHT = 0x14,
    HARD_CYAN_LIGHT = 0xD,
    LIGHT_BLUE_LIGHT = 0x11,
    BLUE_LIGHT = 0x11,
    LIGHT_PURPLE_LIGHT = 0x16,
    FLASH = 0xB,
    FADE = 0x13,
    SMOOTH = 0x17,
};


class LEDTransmitter : IRTransmitter
{
public:
    explicit LEDTransmitter(gpio_num_t gpio_num);
    ~LEDTransmitter() = default;

public:
    void send(const IRCommand& cmd) override;
    void send(const LEDProtocol& cmd);
    void send_multiple(const LEDProtocol& cmd, const uint16_t amount);
};
