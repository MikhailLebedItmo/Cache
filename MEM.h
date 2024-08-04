// Класс моделирующий оперативную память.

#pragma once

#include "MEMLine.h"
#include "AddressConfig.h"

template <uint8_t TagLen, uint8_t IndexLen, uint8_t OffsetLen>
class MEM {
private:
    using address_config = AddressConfig<TagLen, IndexLen, OffsetLen>;
    using mem_line = MEMLine<address_config::max_tag, address_config::max_offset>;
public:
    MEM() : bytes(address_config::max_address) {

    }

    // Возвращает линию оперативной памяти, в которой находиться ячейка с данным адрессом
    mem_line read_line(uint32_t address) {
        auto [tag, index, offset] = address_config::split_address(address);
        auto [first_cell_index, last_cell_index] = address_config::get_containing_line(address);
        auto first_cell = bytes.begin() + first_cell_index;
        return {tag, std::span<std::byte>(first_cell, address_config::max_offset)};
    }
private:
    std::vector<std::byte> bytes;
};



