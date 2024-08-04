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
