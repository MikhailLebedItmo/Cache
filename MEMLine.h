// Вся оперативная память разбиваться на линии - минимальное количество ячеек которые можно считать.
// Данный класс моделирует одну такую линию.

#pragma once

#include <cstdint>
#include <vector>
#include <ranges>
#include <cassert>

template <uint32_t MaxTag, uint32_t LineSize>
class MEMLine {
public:
    MEMLine() = default;

    MEMLine(uint32_t tag, std::span<std::byte> line)
            : tag(tag)
            , line(line) {
        assert(tag < MaxTag);
        assert(line.size() == LineSize);
    }

    std::byte& get_mem_cell(uint32_t pos) {
        assert(pos < LineSize);
        return line[pos];
    }

    uint32_t get_tag() const {
        return tag;
    }
protected:
    uint32_t tag = MaxTag;
    std::span<std::byte> line;
};
