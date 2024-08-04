#include "ValueExpander.h"

uint32_t ValueExpander::convert_to_unsigned_value(const std::string& value) {
    uint32_t ivalue;
    size_t pend = 0;
    if (value.size() > 2 && value[1] == 'x') {
        ivalue = std::stoul(value, &pend, 16);
    } else {
        ivalue = std::stoul(value, &pend, 10);
    }
    if (pend != value.size()) {
        std::cerr << "Can't convert " << value << " to numeric value" << std::endl;
        exit(EXIT_FAILURE);
    }
    return ivalue;
}

template<uint8_t N>
int32_t ValueExpander::expand_N_bit_value(const std::string& value) {
    static_assert(N < 32);
    uint32_t ivalue = convert_to_unsigned_value(value);
    return expand_N_bit_value<N>(ivalue);
}

template<uint8_t N>
int32_t ValueExpander::expand_N_bit_value(uint32_t ivalue) {
    static_assert(N < 32);
    if ((ivalue >> (N - 1)) & 1) {
        // число отрицательное => добавляем 1(биты) в начало числа
        uint32_t mask = (1 << (32 - N)) - 1;
        mask <<= N;
        ivalue |= mask;
    }
    return static_cast<int32_t>(ivalue);
}
