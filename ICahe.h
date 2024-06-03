#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>


template <typename Implementation>
class ICache {
    ICache() = default;

    template<uint8_t BytesCnt>
    uint32_t read(uint32_t address) {
        static_assert(BytesCnt < 4 && BytesCnt > 0);
        ++requests_cnt;
        uint32_t bytes_read = 0;
        bool is_hit = true;
        for (uint8_t i = 0; i < BytesCnt; ++i) {
            auto res = get_mem_cell(address);  // todo: rename
            is_hit = is_hit && res.second;
            bytes_read |= res.first << (i * 8);
        }
        hits_cnt += is_hit;

        return bytes_read;
    }

    template<uint8_t BytesCnt>
    void write(uint32_t address, uint32_t value) {
        static_assert(BytesCnt < 4 && BytesCnt > 0);
        ++requests_cnt;
        bool is_hit = true;
        uint32_t byte_mask = (1 << 8) - 1;
        for (uint8_t i = 0; i < BytesCnt; ++i) {
            auto res = get_mem_cell(address);
            is_hit = is_hit && res.second;
            res.first = static_cast<std::byte>(value & byte_mask);
            value >>= 8;
        }
        hits_cnt += is_hit;
    }

    uint32_t get_requests_cnt() {
        return requests_cnt;
    }

    uint32_t get_hits_cnt() {
        return hits_cnt;
    }
private:
    std::pair<std::byte&, bool> get_mem_cell(uint32_t address) {
        return impl()->get_mem_cell(address);
    }

    Implementation* impl() {
        return static_cast<Implementation*>(this);
    }
private:
    uint32_t requests_cnt = 0;
    uint32_t hits_cnt = 0;
};