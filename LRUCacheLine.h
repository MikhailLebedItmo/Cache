// Моделирует одну кэш линию LRU-кэша.
// Хранит flag обозначающий, номер в очереди на удаление

#pragma once

#include "MEMLine.h"

#include <cassert>

template <uint32_t MaxTag, uint32_t LineSize>
class LRUCacheLine : private MEMLine<MaxTag, LineSize> {
public:
    using MEMLine<MaxTag, LineSize>::get_mem_cell;
    using MEMLine<MaxTag, LineSize>::get_tag;

    LRUCacheLine() = default;

    void update_line(const MEMLine<MaxTag, LineSize>& mem_line) {
        MEMLine<MaxTag, LineSize>::operator=(mem_line);
    }

    uint32_t& flag() {
        return flag_;
    }

    const uint32_t& flag() const {
        return flag_;
    }

    // В начале работы программы все линии кэша находятся в невалидном состоянии
    bool is_invalidated() const {
        return MEMLine<MaxTag, LineSize>::line.size() == 0;
    }
private:
    uint32_t flag_ = 0;
};


