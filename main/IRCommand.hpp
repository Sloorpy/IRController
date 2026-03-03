#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class SignalState : uint8_t
{
    NORMAL = 0,
    REPEAT = 1,
    INVALID = 2
};

struct IRCommand
{
    SignalState state;
    uint8_t address;
    uint8_t command;

    std::string str() const;
};

class NECProtocol
{
public:
    static constexpr uint16_t NEC_ESTIMATED_ERROR = 500;

    static constexpr uint16_t NEC_LEADER_BURST_US = 9000;
    static constexpr uint16_t NEC_LEADER_SPACE_US = 4500;
    
    static constexpr uint16_t NEC_BIT_BURST_US = 562;
    static constexpr uint16_t NEC_BIT_0_SPACE_US = 560;
    static constexpr uint16_t NEC_BIT_1_SPACE_US = 1690;

    static constexpr uint16_t NEC_REPEAT_SPACE_US = 2250;
    
public:
    static std::vector<uint16_t> encode(uint8_t address, uint8_t command);

    static std::vector<uint16_t> encode_led(uint8_t command);

    static IRCommand decode(const std::vector<uint16_t>& timings);

    static bool is_repeat(const std::vector<uint16_t>& timings);

    static bool in_range(const uint16_t time, const uint16_t desired_val);

    static bool get_bit(const std::vector<uint16_t>& timings, const uint32_t index);

    static std::string timings_str(const std::vector<uint16_t>& timings);

private:
    static uint8_t invert(uint8_t value);
};
