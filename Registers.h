#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

class Registers {
private:
    static const std::vector<std::string> registers_names_1; // todo rename
    static const std::vector<std::string> registers_names_2;
public:
    Registers();

    uint32_t& operator[] (const std::string& key);
private:
    std::vector<uint32_t> values;
    std::unordered_map<std::string, std::vector<uint32_t>::iterator> registers;
};