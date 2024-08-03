#pragma once

#include "MEMLine.h"

#include <cassert>

template <uint32_t MaxTag, uint32_t LineSize>
class PLRUCacheLine : private MEMLine<MaxTag, LineSize> {
public:
    using MEMLine<MaxTag, LineSize>::get_mem_cell;
    using MEMLine<MaxTag, LineSize>::get_tag;

    PLRUCacheLine() = default;

    void update_line(const MEMLine<MaxTag, LineSize>& mem_line) {
        MEMLine<MaxTag, LineSize>::operator=(mem_line);
    }

    bool& flag() {
        return flag_;
    }

    const bool& flag() const {
        return flag_;
    }

    // At program startup all lines are in invalid state
    bool is_invalidated() const {
        return this->tag == MaxTag;
    }
private:
    bool flag_ = false;
};