#pragma once

#include "ICahe.h"
#include "Registers.h"

#include <unordered_map>
#include <vector>


struct TwoArgs {
    std::string& arg_1;
    std::string& arg_2;
};

struct ThreeArgs : public TwoArgs {
    std::string& arg_3;
};


template <typename Implementation>
class Interpreter {
public:

private:
    using Commands = std::vector<std::vector<std::string>>;
    using TwoArgsInstruction = void (Interpreter::*)(TwoArgs);
    using ThreeArgsInstruction = void (Interpreter::*)(ThreeArgs);
    static const std::unordered_map<std::string, TwoArgsInstruction> two_args_instructions;
    static const std::unordered_map<std::string, ThreeArgsInstruction> three_args_instructions;
    Commands commands;
    Commands::iterator cur_command;
    Registers registers;
    ICache<Implementation>& cache;
};