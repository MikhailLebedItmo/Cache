// Класс исполняющий ассемблерный код

// Для исполнения каждой ассемблерной команды есть соответсвующий метод, указатели на которые
// лежат в статических хэш-мапах: two_args_instructions и three_args_instructions.

#pragma once

#include "ICahe.h"
#include "Registers.h"
#include "AsmCommandArgs.h"
#include "ValueExpander.h"

#include <unordered_map>
#include <vector>
#include <iostream>


template <typename CacheImplementation>
class Interpreter {
public:
    using Commands = std::vector<std::vector<std::string>>;
private:
    using TwoArgsInstruction = void (Interpreter::*)(TwoArgsCommandArgs&);
    using ThreeArgsInstruction = void (Interpreter::*)(ThreeArgsCommandArgs&);
public:
    struct HitsStatistics {
        uint32_t hits_cnt;
        uint32_t requests_cnt;
    };

    Interpreter(const Commands& code, ICache<CacheImplementation>& cache)
        : code(code)
        , cur_command(this->code.begin())
        , registers()
        , cache(cache) {

    }

    HitsStatistics run() {
        while (cur_command->front() != "jalr" || ((*cur_command)[2] != "ra" && (*cur_command)[2] != "x1")) {
            if (cur_command->size() == 3) {
                TwoArgsCommandArgs args = {(*cur_command)[1], (*cur_command)[2]};
                auto instruction_ptr = two_args_instructions.at(cur_command->front());
                (*this.*instruction_ptr)(args);
            } else if (cur_command->size() == 4) {
                ThreeArgsCommandArgs args{
                        {(*cur_command)[1], (*cur_command)[2]}, (*cur_command)[3]
                };
                auto instruction_ptr = three_args_instructions.at(cur_command->front());
                (*this.*instruction_ptr)(args);
            } else {
                std::cerr << "Error: wrong command";
                exit(EXIT_FAILURE);
            }
        }

        return {cache.get_hits_cnt(), cache.get_requests_cnt()};
    }
private: // Instructions
    void lui(TwoArgsCommandArgs& args) {
        uint32_t val = ValueExpander::convert_to_unsigned_value(args.arg_2) << 11;
        registers[args.arg_1] = val;
        jump(1);
    }
    void auipc(TwoArgsCommandArgs& args) {
        uint32_t address = registers["pc"] + (ValueExpander::convert_to_unsigned_value(args.arg_2) << 11);
        registers[args.arg_1] = address;
        jump(1);
    }

    void jal(TwoArgsCommandArgs& args) {
        int shift = ValueExpander::expand_N_bit_value<21>(args.arg_2) / 4;
        registers[args.arg_1] = registers["pc"] + 4;
        jump(shift);
    }

    void mul(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = (int)registers[args.arg_2] * (int)registers[args.arg_3];
        jump(1);
    }

    void mulh(ThreeArgsCommandArgs& args) {
        uint64_t res = (int64_t)registers[args.arg_2] * (int64_t)registers[args.arg_3];
        registers[args.arg_1] = res >> 32;
        jump(1);
    }

    void mulhsu(ThreeArgsCommandArgs& args) {
        uint64_t res = (uint64_t)registers[args.arg_2] * (uint64_t)registers[args.arg_3];
        registers[args.arg_1] = res >> 32;
        jump(1);
    }

    void mulhu(ThreeArgsCommandArgs& args) {
        uint64_t res = (int64_t)registers[args.arg_2] * (uint64_t)registers[args.arg_3];
        registers[args.arg_1] = res >> 32;
        jump(1);
    }

    void div(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = (int)registers[args.arg_2] / (int)registers[args.arg_3];
        jump(1);
    }

    void divu(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] / registers[args.arg_3];
        jump(1);
    }

    void rem(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = (int)registers[args.arg_2] % (int)registers[args.arg_3];
        jump(1);
    }

    void remu(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] % registers[args.arg_3];
        jump(1);
    }

    void jalr(ThreeArgsCommandArgs& args) {
        uint32_t new_address = registers[args.arg_2] + ValueExpander::expand_N_bit_value<12>(args.arg_3);
        new_address = new_address << 1 >> 1;
        registers[args.arg_1] = registers["pc"] + 4;
        int shift = ((int)new_address - (int)registers["pc"]) / 4;
        jump(shift);
    }

    void beq(ThreeArgsCommandArgs& args) {
        if (registers[args.arg_1] == registers[args.arg_2]) {
            return jump(ValueExpander::expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void bne(ThreeArgsCommandArgs& args) {
        if (registers[args.arg_1] != registers[args.arg_2]) {
            return jump(ValueExpander::expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void blt(ThreeArgsCommandArgs& args) {
        if ((int)registers[args.arg_1] < (int)registers[args.arg_2]) {
            return jump(ValueExpander::expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void bge(ThreeArgsCommandArgs& args) {
        if ((int)registers[args.arg_1] >= (int)registers[args.arg_2]) {
            return jump(ValueExpander::expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void bltu(ThreeArgsCommandArgs& args) {
        if ((uint32_t)registers[args.arg_1] < (uint32_t)registers[args.arg_2]) {
            return jump(ValueExpander::expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void bgeu(ThreeArgsCommandArgs& args) {
        if ((uint32_t)registers[args.arg_1] >= (uint32_t)registers[args.arg_2]) {
            return jump(ValueExpander::expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void lb(ThreeArgsCommandArgs& args) {
        uint32_t address = registers[args.arg_3] + ValueExpander::expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = ValueExpander::expand_N_bit_value<8>(cache.template read<1>(address));
        jump(1);
    }

    void lh(ThreeArgsCommandArgs& args) {
        uint32_t address = registers[args.arg_3] + ValueExpander::expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = ValueExpander::expand_N_bit_value<16>(cache.template read<2>(address));
        jump(1);
    }

    void lw(ThreeArgsCommandArgs& args) {
        uint32_t address = registers[args.arg_3] + ValueExpander::expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.template read<4>(address);
        jump(1);
    }

    void lbu(ThreeArgsCommandArgs& args) {
        uint32_t address = registers[args.arg_3] + ValueExpander::expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.template read<1>(address);
        jump(1);
    }

    void lhu(ThreeArgsCommandArgs& args) {
        uint32_t address = registers[args.arg_3] + ValueExpander::expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.template read<2>(address);
        jump(1);
    }

    void sb(ThreeArgsCommandArgs& args) {
        uint32_t address = registers[args.arg_3] + ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t byte_mask = (1 << 8) - 1;
        uint8_t byte = registers[args.arg_1] & byte_mask;
        cache.template write<1>(address, byte);
        jump(1);
    }

    void sh(ThreeArgsCommandArgs& args) {
        uint32_t address = registers[args.arg_3] + ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t half_word_mask = (1 << 16) - 1;
        uint16_t half_word = registers[args.arg_1] & half_word_mask;
        cache.template write<2>(address, half_word);
        jump(1);
    }

    void sw(ThreeArgsCommandArgs& args) {
        uint32_t address = registers[args.arg_3] + ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t word = registers[args.arg_1];
        cache.template write<4>(address, word);
        jump(1);
    }

    void addi(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] + ValueExpander::expand_N_bit_value<12>(args.arg_3);
        jump(1);
    }

    void slti(ThreeArgsCommandArgs& args) {
        if ((int)registers[args.arg_2] < ValueExpander::expand_N_bit_value<12>(args.arg_3)) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void sltiu(ThreeArgsCommandArgs& args) {
        if (registers[args.arg_2] < ValueExpander::convert_to_unsigned_value(args.arg_3)) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void xori(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] ^ ValueExpander::expand_N_bit_value<12>(args.arg_3);
        jump(1);
    }

    void ori(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] | ValueExpander::expand_N_bit_value<12>(args.arg_3);
        jump(1);
    }

    void andi(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] & ValueExpander::expand_N_bit_value<12>(args.arg_3);
        jump(1);
    }

    void slli(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] << ValueExpander::convert_to_unsigned_value(args.arg_3);
        jump(1);
    }

    void srli(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] >> ValueExpander::convert_to_unsigned_value(args.arg_3);
        jump(1);
    }

    void srai(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = (int)registers[args.arg_2] >> ValueExpander::convert_to_unsigned_value(args.arg_3);
        jump(1);
    }

    void add(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] + registers[args.arg_3];
        jump(1);
    }

    void sub(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] - registers[args.arg_3];
        jump(1);
    }

    void sll(ThreeArgsCommandArgs& args) {
        uint32_t shift_mask = (1 << 5) - 1;
        uint32_t shift = registers[args.arg_3] & shift_mask;
        registers[args.arg_1] = registers[args.arg_2] << shift;
        jump(1);
    }

    void slt(ThreeArgsCommandArgs& args) {
        if ((int)registers[args.arg_2] < (int)registers[args.arg_3]) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void sltu(ThreeArgsCommandArgs& args) {
        if (registers[args.arg_2] < registers[args.arg_3]) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void xor_(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] ^ registers[args.arg_3];
        jump(1);
    }

    void srl(ThreeArgsCommandArgs& args) {
        uint32_t shift_mask = (1 << 5) - 1;
        uint32_t shift = registers[args.arg_3] & shift_mask;
        registers[args.arg_1] = registers[args.arg_2] >> shift;
        jump(1);
    }

    void sra(ThreeArgsCommandArgs& args) {
        uint32_t shift_mask = (1 << 5) - 1;
        uint32_t shift = registers[args.arg_3] & shift_mask;
        registers[args.arg_1] = (int)registers[args.arg_2] >> shift;
        jump(1);
    }

    void or_(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] | registers[args.arg_3];
        jump(1);
    }

    void and_(ThreeArgsCommandArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] & registers[args.arg_3];
        jump(1);
    }

private:
    // вспомогательный метод для перемещения по коду
    void jump(int shift) {
        cur_command += shift;
        registers["pc"] += 4 * shift;
    }
private:
    inline static const std::unordered_map<std::string, TwoArgsInstruction> two_args_instructions = {
            {"lui", &Interpreter::lui}, {"auipc", &Interpreter::auipc},
            {"jal", &Interpreter::jal}
    };
    inline static const std::unordered_map<std::string, ThreeArgsInstruction> three_args_instructions = {
            {"jalr", &Interpreter::jalr}, {"beq", &Interpreter::beq}, {"bne", &Interpreter::bne},
            {"blt", &Interpreter::blt}, {"bge", &Interpreter::bge}, {"bltu", &Interpreter::bltu},
            {"bgeu", &Interpreter::bgeu}, {"lb", &Interpreter::lb}, {"lh", &Interpreter::lh},
            {"lw", &Interpreter::lw},
            {"lbu", &Interpreter::lbu}, {"lhu", &Interpreter::lhu}, {"sb", &Interpreter::sb},
            {"sh", &Interpreter::sh},
            {"sw", &Interpreter::sw}, {"addi", &Interpreter::addi}, {"slti", &Interpreter::slti},
            {"sltiu", &Interpreter::sltiu}, {"xori", &Interpreter::xori}, {"ori", &Interpreter::ori},
            {"andi", &Interpreter::andi}, {"slli", &Interpreter::slli}, {"srli", &Interpreter::srli},
            {"srai", &Interpreter::srai}, {"add", &Interpreter::add}, {"sub", &Interpreter::sub},
            {"sll", &Interpreter::sll},{"slt", &Interpreter::slt}, {"sltu", &Interpreter::sltu},
            {"xor", &Interpreter::xor_},
            {"srl", &Interpreter::srl}, {"sra", &Interpreter::sra}, {"or", &Interpreter::or_},
            {"and", &Interpreter::and_},
            {"mul", &Interpreter::mul}, {"mulh", &Interpreter::mulh}, {"mulhsu", &Interpreter::mulhsu},
            {"mulhu", &Interpreter::mulhu}, {"div", &Interpreter::div}, {"divu", &Interpreter::divu},
            {"rem", &Interpreter::rem}, {"remu", &Interpreter::remu}
    };
    const Commands& code;
    Commands::const_iterator cur_command;
    Registers registers;
    ICache<CacheImplementation>& cache;
};