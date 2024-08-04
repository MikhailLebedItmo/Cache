#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <bitset>

class InstructionEncoder {
public:
    InstructionEncoder& add_bits(uint8_t bits_cnt, uint32_t bits);

    InstructionEncoder& add_register(const std::string& reg_name);

    uint32_t get_code();
private:
    static const std::unordered_map<std::string, uint8_t> from_reg_name_to_code;
    std::bitset<32> bin_code;
    uint8_t cur_index = 32;
};