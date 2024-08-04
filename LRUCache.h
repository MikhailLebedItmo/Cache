// Класс моделирующий процессорный кэш с политикой вытеснения Least Recently Used(LRU)

#pragma once

#include "LRUCacheLines.h"
#include "MEM.h"
#include "ICahe.h"
#include "CacheLookupResult.h"

#include <cassert>

template <uint32_t CacheWaysCnt, uint8_t TagLen, uint8_t IndexLen, uint8_t OffsetLen>
class LRUCache : public ICache<LRUCache<CacheWaysCnt, TagLen, IndexLen, OffsetLen>> {  // use CRTP idiom
private:
    using address_config_t = AddressConfig<TagLen, IndexLen, OffsetLen>;
    using lru_cashe_lines_t = LRUCacheLines<CacheWaysCnt, TagLen, IndexLen, OffsetLen>;
    using mem_t = MEM<TagLen, IndexLen, OffsetLen>;
public:
    LRUCache(): cache(address_config_t::max_index) {

    }

    CacheLookupResult get_mem_cell(uint32_t address) {
        assert(address < address_config_t::max_address);
        auto [tag, index, offset] = address_config_t::split_address(address);
        return cache[index].get_mem_cell(address, mem);
    }
private:
    std::vector<lru_cashe_lines_t> cache;
    mem_t mem;
};
