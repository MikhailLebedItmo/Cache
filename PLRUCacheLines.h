#pragma once

#include "MEM.h"
#include "MEMLine.h"
#include "AddressConfig.h"
#include "PLRUCacheLine.h"
#include "CacheLookupResult.h"

#include <cstdint>
#include <algorithm>

template <uint32_t CacheWaysCnt, uint8_t TagLen, uint8_t IndexLen, uint8_t OffsetLen>
class PLRUCacheLines {
private:
    using address_config_t = AddressConfig<TagLen, IndexLen, OffsetLen>;
    using plru_cashe_line_t = PLRUCacheLine<address_config_t::max_tag, address_config_t::max_offset>;
    using mem_t = MEM<TagLen, IndexLen, OffsetLen>;
public:
    PLRUCacheLines()
            : lines(CacheWaysCnt) {

    }

    // Возвращает ссылку на ячейку оперативной памяти. Если её не было в кэше - предварительно загружает.
    CacheLookupResult get_mem_cell(uint32_t address, mem_t& mem) {
        auto [tag, index, offset] = address_config_t::split_address(address);
        auto it = std::ranges::find_if(lines,
                                       [tag](const auto& line){return line.get_tag() == tag;}
        );
        if (it != lines.end()) {
            // Ячейка есть в кэше
            update_flags(it);

            return {it->get_mem_cell(offset), true};
        }
        auto delete_line_it = std::ranges::find_if(lines,
                                  [](const auto& line){return line.flag() == 0;}
        );
        delete_line_it->update_line(mem.read_line(address));
        update_flags(delete_line_it);

        return {delete_line_it->get_mem_cell(offset), false};
    }
private:
    std::vector<plru_cashe_line_t> lines;

    void update_flags(std::vector<plru_cashe_line_t>::iterator used_line) {
        used_line->flag() = true;
        auto zero_flag_line = std::ranges::find_if(lines, [](plru_cashe_line_t& line){return !line.flag();});
        if (zero_flag_line != lines.end()) {
            return;
        }
        // если все флаги стали true, нужно обнулить их
        auto update_flag = [](plru_cashe_line_t& line) {
            line.flag() = false;
        };
        std::ranges::for_each(lines, update_flag);
        used_line->flag() = true;
    }
};
