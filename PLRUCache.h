#pragma once

#pragma once

#include "PLRUCacheLines.h"
#include "MEM.h"
#include "ICahe.h"

#include <cassert>

template <uint32_t CacheWaysCnt, uint8_t TagLen, uint8_t IndexLen, uint8_t OffsetLen>
class PLRUCache: public ICache<PLRUCache<CacheWaysCnt, TagLen, IndexLen, OffsetLen>> { // use CRTP idiom
private:
    using address_config_t = AddressConfig<TagLen, IndexLen, OffsetLen>;
    using lru_cashe_lines_t = PLRUCacheLines<CacheWaysCnt, TagLen, IndexLen, OffsetLen>;
    using mem_t = MEM<TagLen, IndexLen, OffsetLen>;
public:
    PLRUCache(): cache(address_config_t::max_index) {

    }

    std::pair<std::byte&, bool> get_mem_cell(uint32_t address) {
        assert(address < address_config_t::max_address);
        auto [tag, index, offset] = address_config_t::split_address(address);
        return cache[index].get_mem_cell(address, mem);
    }
private:
    std::vector<lru_cashe_lines_t> cache;
    mem_t mem;
};
