#pragma once

#include <cstdint>
#include <tuple>

template <uint8_t TagLen, uint8_t IndexLen, uint8_t OffsetLen>
class AddressConfig {
public:
    static constexpr uint32_t max_tag  = (uint32_t)1 << TagLen;
    static constexpr uint32_t max_index  = (uint32_t)1 << IndexLen;
    static constexpr uint32_t max_offset  = (uint32_t)1 << OffsetLen;
    static constexpr uint32_t max_address = (uint32_t)1 << (TagLen + IndexLen + OffsetLen);

    static std::tuple<uint32_t, uint32_t, uint32_t> split_address(uint32_t address) {
        uint32_t tag = address >> (IndexLen + OffsetLen);
        uint32_t index = (address >> OffsetLen) & index_mask;
        uint32_t offset = address & offset_mask;

        return {tag, index, offset};
    }

    static std::pair<uint32_t, uint32_t> get_containing_line(uint32_t address) {
        uint32_t first_cell_index = address >> OffsetLen << OffsetLen;
        uint32_t last_cell_index = first_cell_index + (1 << OffsetLen);

        return {first_cell_index, last_cell_index};
    }
private:
    static constexpr uint32_t index_mask = ((uint32_t)1 << IndexLen) - 1;
    static constexpr uint32_t offset_mask = ((uint32_t)1 << OffsetLen) - 1;
};

