#pragma once

#include <cstddef>

struct CacheLookupResult {
    std::byte& cell;
    bool is_hit;
};