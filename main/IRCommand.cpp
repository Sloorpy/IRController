#include "IRCommand.hpp"
#include "Exception.hpp"

#include <cstdint>
#include <cstdio>
#include <ctime>
#include "esp_log.h"


std::string IRCommand::str() const
{
    if (this->state == SignalState::REPEAT)
    {
        return std::string("Repeat code");
    }
    
    if (this->state == SignalState::INVALID)
    {
        return std::string("Invalid signal");
    }

    char buffer[30];
    snprintf(buffer, sizeof(buffer), "addr=0x%02X cmd=0x%02X", this->address, this->command);
    return std::string(buffer);
}

std::string NECProtocol::timings_str(const std::vector<uint16_t>& raw_timings)
{
    char buffer[16];
    std::string result = "Raw timings (" + std::to_string(raw_timings.size()) + "): ";
    for (size_t i = 0; i < raw_timings.size(); i++) {
        snprintf(buffer, sizeof(buffer), "%u, ", raw_timings[i]);
        result += buffer;
    }

    result.pop_back();
    return result;
}

std::vector<uint16_t> NECProtocol::encode(uint8_t address, uint8_t command)
{
    std::vector<uint16_t> timings;
    timings.reserve(67);

    timings.push_back(NEC_LEADER_BURST_US);
    timings.push_back(NEC_LEADER_SPACE_US);

    uint8_t inv_address = invert(address);
    uint8_t inv_command = invert(command);

    auto add_bit = [&](bool bit) {
        timings.push_back(NEC_BIT_BURST_US);
        timings.push_back(bit ? NEC_BIT_1_SPACE_US : NEC_BIT_0_SPACE_US);
    };

    for (int i = 0; i < 8; i++) {
        add_bit(address & (1 << i));
    }
    for (int i = 0; i < 8; i++) {
        add_bit(inv_address & (1 << i));
    }
    for (int i = 0; i < 8; i++) {
        add_bit(command & (1 << i));
    }
    for (int i = 0; i < 8; i++) {
        add_bit(inv_command & (1 << i));
    }

    timings.push_back(NEC_BIT_BURST_US);

    return timings;
}

std::vector<uint16_t> NECProtocol::encode_led(uint8_t command)
{
    std::vector<uint16_t> timings;
    timings.reserve(67);

    timings.push_back(NEC_LEADER_BURST_US);
    timings.push_back(NEC_LEADER_SPACE_US);

    uint8_t inv_command = invert(command);

    auto add_bit = [&](bool bit) {
        timings.push_back(NEC_BIT_BURST_US);
        timings.push_back(bit ? NEC_BIT_1_SPACE_US : NEC_BIT_0_SPACE_US);
    };

    for (int i = 0; i < 8; i++) {
        add_bit(0);
    }
    for (int i = 0; i < 4; i++) {
        add_bit(1);
    }
    add_bit(0);
    for (int i = 0; i < 3; i++) {
        add_bit(1);
    }

    for (int i = 0; i < 8; i++) {
        add_bit(command & (1 << i));
    }
    for (int i = 0; i < 8; i++) {
        add_bit(inv_command & (1 << i));
    }

    timings.push_back(NEC_BIT_BURST_US);

    return timings;
}

bool NECProtocol::get_bit(const std::vector<uint16_t>& timings, const uint32_t index)
{
    if (timings.size() < index * 2 + 1)
    {
        throw Exception(ErrorCode::INDEX_OUT_OF_RANGE);
    }
    return in_range(timings[index * 2], NEC_BIT_BURST_US) && in_range(timings[index * 2 + 1], NEC_BIT_1_SPACE_US);
}


IRCommand NECProtocol::decode(const std::vector<uint16_t>& timings)
{   
    if (is_repeat(timings))
    {
        return IRCommand{.state = SignalState::REPEAT};
    }

    static constexpr uint32_t NEC_PULSE_SIZE = 67;
    if (timings.size() < NEC_PULSE_SIZE) 
    {
        throw IRException(IRErrorCode::NOT_NEC_PROTOCOL);
    }

    static constexpr uint32_t LEADER_BURST_INDEX = 0;
    static constexpr uint32_t LEADER_SPACE_INDEX = 1;
    if (!in_range(timings[LEADER_BURST_INDEX], NEC_LEADER_BURST_US) || !in_range(timings[LEADER_SPACE_INDEX], NEC_LEADER_SPACE_US))
    {
        throw IRException(IRErrorCode::NOT_NEC_PROTOCOL);
    }

    
    static constexpr uint32_t DATA_OFFSET = 1;

    uint8_t address_bits = 0;
    for (uint32_t i = 0; i < 8; i++) {
        if (get_bit(timings, i + DATA_OFFSET)) {
            address_bits |= (1 << i);
        }
    }

    uint8_t inv_address_bits = 0;
    for (uint32_t i = 0; i < 8; i++) {
        if (get_bit(timings, 8 + i + DATA_OFFSET)) {
            inv_address_bits |= (1 << i);
        }
    }

    uint8_t command_bits = 0;
    for (uint32_t i = 0; i < 8; i++) {
        if (get_bit(timings,16 + i + DATA_OFFSET)) {
            command_bits |= (1 << i);
        }
    }

    uint8_t inv_command_bits = 0;
    for (uint32_t i = 0; i < 8; i++) {
        if (get_bit(timings, 24 + i + DATA_OFFSET)) {
            inv_command_bits |= (1 << i);
        }
    }

    // LED Protocol is stupid, also check if it passes 
    if (invert(address_bits) != inv_address_bits && invert(address_bits) != inv_address_bits +  0x10) 
    {
        throw IRException(IRErrorCode::ADDRESSES_DONT_MATCH);
    }
    if (invert(command_bits) != inv_command_bits) 
    {
        throw IRException(IRErrorCode::COMMANDS_DONT_MATCH);
    }

    return IRCommand{
        .state = SignalState::NORMAL,
        .address = address_bits,
        .command = command_bits,
    };
}

bool NECProtocol::in_range(const uint16_t time, const uint16_t desired_val)
{
    return time <= desired_val + NEC_ESTIMATED_ERROR && time >= desired_val - NEC_ESTIMATED_ERROR;
}

bool NECProtocol::is_repeat(const std::vector<uint16_t>& timings)
{
    return timings.size() == 3 &&
            in_range(timings[0], NEC_LEADER_BURST_US) &&
            in_range(timings[1], NEC_REPEAT_SPACE_US) && 
            in_range(timings[2], NEC_BIT_BURST_US);
}

uint8_t NECProtocol::invert(uint8_t value)
{
    return static_cast<uint8_t>(~value);
}
