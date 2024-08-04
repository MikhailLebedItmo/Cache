// Вспомогательный класс для перевода чисел из N-битной формы в 32 битную форму(с учётом знака)

#pragma once

#include <cstdint>
#include <iostream>
#include <string>


class ValueExpander {
public:
    static uint32_t convert_to_unsigned_value(const std::string& value);

    template <uint8_t N>
    static int32_t expand_N_bit_value(uint32_t ivalue);

    template <uint8_t N>
    static int32_t expand_N_bit_value(const std::string& value);
};

