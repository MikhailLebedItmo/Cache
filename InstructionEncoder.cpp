#include "InstructionEncoder.h"

const std::unordered_map<std::string, uint8_t> InstructionEncoder::from_reg_name_to_code = {
    {"x0", 0}, {"x1", 1}, {"x2", 2}, {"x3", 3}, {"x4", 4}, {"x5", 5}, {"x6", 6}, {"x7", 7}, {"x8", 8}, {"x9", 9},
    {"x10", 10}, {"x11", 11}, {"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15}, {"x16", 16}, {"x17", 17},
    {"x18", 18}, {"x19", 19}, {"x20", 20}, {"x21", 21}, {"x22", 22}, {"x23", 23}, {"x24", 24}, {"x25", 25},
    {"x26", 26}, {"x27", 27}, {"x28", 28}, {"x29", 29}, {"x30", 30}, {"x31", 31}, {"zero", 0}, {"ra", 1},
    {"sp", 2}, {"gp", 3}, {"tp", 4}, {"t0", 5}, {"t1", 6}, {"t2", 7}, {"s0", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11},
    {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19}, {"s4", 20},
    {"s5", 21}, {"s6", 22}, {"s7", 23}, {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
    {"t5", 30}, {"t6", 31}, {"fp", 8}
};

InstructionEncoder& InstructionEncoder::add_bits(uint8_t bits_cnt, uint32_t bits) {
    while(bits_cnt--) {
        bin_code.set(--cur_index, (bits >> bits_cnt) & 1);
    }
    return *this;
}

InstructionEncoder& InstructionEncoder::add_register(const std::string& reg_name) {
    uint8_t reg_code = InstructionEncoder::from_reg_name_to_code.at(reg_name);
    add_bits(5, reg_code);
    return *this;
}

uint32_t InstructionEncoder::get_code() {
    return bin_code.to_ulong();
}


