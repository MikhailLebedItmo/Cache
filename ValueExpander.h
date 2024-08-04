// Вспомогательный класс для перевода чисел из N-битной формы в 32 битную форму(с учётом знака)

#pragma once

#include <cstdint>
#include <iostream>
#include <string>


class ValueExpander {
public:
    static uint32_t convert_to_unsigned_value(const std::string& value);

    template <uint8_t N>
    static int32_t expand_N_bit_value(uint32_t ivalue) {
        static_assert(N < 32);
        if ((ivalue >> (N - 1)) & 1) {
            // число отрицательное => добавляем 1(биты) в начало числа
            uint32_t mask = (1 << (32 - N)) - 1;
            mask <<= N;
            ivalue |= mask;
        }
        return static_cast<int32_t>(ivalue);
    }

    template <uint8_t N>
    static int32_t expand_N_bit_value(const std::string& value) {
        static_assert(N < 32);
        uint32_t ivalue = convert_to_unsigned_value(value);
        return expand_N_bit_value<N>(ivalue);
    }
};

