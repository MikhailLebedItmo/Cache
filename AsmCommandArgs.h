#pragma once

#include <string>

struct TwoArgsCommandArgs {
    const std::string& arg_1;
    const std::string& arg_2;
};

struct ThreeArgsCommandArgs : public TwoArgsCommandArgs{
    const std::string& arg_3;
};
