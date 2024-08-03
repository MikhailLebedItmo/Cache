#pragma once

#include "LRUCacheLine.h"
#include "AddressConfig.h"
#include "MEM.h"

#include <cstdint>
#include <vector>
#include <algorithm>

template <uint32_t CacheWaysCnt, uint8_t TagLen, uint8_t IndexLen, uint8_t OffsetLen>
class LRUCacheLines {
private:
    using address_config_t = AddressConfig<TagLen, IndexLen, OffsetLen>;
    using lru_cashe_line_t = LRUCacheLine<address_config_t::max_tag, address_config_t::max_offset>;
    using mem_t = MEM<TagLen, IndexLen, OffsetLen>;
public:
    LRUCacheLines()
        : lines(CacheWaysCnt) {

    }

    // todo: change pair, tuple on structures
    std::pair<std::byte&, bool> get_mem_cell(uint32_t address, mem_t mem) {
        auto [tag, index, offset] = address_config_t::split_address(address);
        auto it = std::ranges::find_if(lines,
            [tag](const auto& line){return line.get_tag() == tag;}
        );
        if (it != lines.end()) {
            update_flags(it);

            return {it->get_mem_cell(offset), true};
        }
        it = std::ranges::find_if(lines,
            [](const auto& line){return line.flag() == 0;}
        );
        it->update_line(mem.read_line(address));
        update_flags(it);

        return {it->get_mem_cell(offset), false};
    }
private:
    std::vector<lru_cashe_line_t> lines;

    void update_flags(std::vector<lru_cashe_line_t>::iterator used_line) {
        auto update_flag = [used_flag = used_line->flag()](lru_cashe_line_t& line) {
            line.flag() = (line.flag() > used_flag) ? line.flag() - 1 : line.flag();
        };
        std::ranges::for_each(lines, update_flag); // updates queue position in LRUCache
        used_line->flag() = lines.size() - 1;
    }
};