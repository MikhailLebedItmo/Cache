#include "Registers.h"


Registers::Registers() {
    values.resize(registers_names_1.size());
    for (int i = 0; i < registers_names_1.size(); ++i) {
        registers[registers_names_1[i]] = values.begin() + i;
    }
    for (int i = 0; i < registers_names_2.size(); ++i) {
        registers[registers_names_2[i]] = values.begin() + i;
    }
    registers["fp"] = values.begin() + 8;
}

uint32_t& Registers::operator[](const std::string& key) {
    *registers["zero"] = 0;
    return *registers[key];
}

const std::vector<std::string> Registers::registers_names_1 = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6", "pc"
};

const std::vector<std::string> Registers::registers_names_2 = {
        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16",
        "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
};



