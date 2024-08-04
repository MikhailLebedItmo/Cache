// Класс исполняющий ассемблерный код

// Для исполнения каждой ассемблерной команды есть соответсвующий метод, указатели на которые
// лежат в статических хэш-мапах: two_args_instructions и three_args_instructions.

#pragma once

#include "ICahe.h"
#include "Registers.h"

#include <unordered_map>
#include <vector>
#include <iostream>

struct TwoArgs {
    const std::string& arg_1;
    const std::string& arg_2;
};

struct ThreeArgs : public TwoArgs {
    const std::string& arg_3;
};


template <typename CacheImplementation>
class Interpreter {
public:
    using Commands = std::vector<std::vector<std::string>>;
private:
    using TwoArgsInstruction = void (Interpreter::*)(TwoArgs&);
    using ThreeArgsInstruction = void (Interpreter::*)(ThreeArgs&);
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
                TwoArgs args = {(*cur_command)[1], (*cur_command)[2]};
                auto instruction_ptr = two_args_instructions.at(cur_command->front());
                (*this.*instruction_ptr)(args);
            } else if (cur_command->size() == 4) {
                ThreeArgs args{
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
    void lui(TwoArgs& args) {
        uint32_t val = convert_to_unsigned_value(args.arg_2) << 11;
        registers[args.arg_1] = val;
        jump(1);
    }
    void auipc(TwoArgs& args) {
        uint32_t address = registers["pc"] + (convert_to_unsigned_value(args.arg_2) << 11);
        registers[args.arg_1] = address;
        jump(1);
    }

    void jal(TwoArgs& args) {
        int shift = expand_N_bit_value<21>(args.arg_2) / 4;
        registers[args.arg_1] = registers["pc"] + 4;
        jump(shift);
    }

    void mul(ThreeArgs& args) {
        registers[args.arg_1] = (int)registers[args.arg_2] * (int)registers[args.arg_3];
        jump(1);
    }

    void mulh(ThreeArgs& args) {
        uint64_t res = (int64_t)registers[args.arg_2] * (int64_t)registers[args.arg_3];
        registers[args.arg_1] = res >> 32;
        jump(1);
    }

    void mulhsu(ThreeArgs& args) {
        uint64_t res = (uint64_t)registers[args.arg_2] * (uint64_t)registers[args.arg_3];
        registers[args.arg_1] = res >> 32;
        jump(1);
    }

    void mulhu(ThreeArgs& args) {
        uint64_t res = (int64_t)registers[args.arg_2] * (uint64_t)registers[args.arg_3];
        registers[args.arg_1] = res >> 32;
        jump(1);
    }

    void div(ThreeArgs& args) {
        registers[args.arg_1] = (int)registers[args.arg_2] / (int)registers[args.arg_3];
        jump(1);
    }

    void divu(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] / registers[args.arg_3];
        jump(1);
    }

    void rem(ThreeArgs& args) {
        registers[args.arg_1] = (int)registers[args.arg_2] % (int)registers[args.arg_3];
        jump(1);
    }

    void remu(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] % registers[args.arg_3];
        jump(1);
    }

    void jalr(ThreeArgs& args) {
        uint32_t new_address = registers[args.arg_2] + expand_N_bit_value<12>(args.arg_3);
        new_address = new_address << 1 >> 1;
        registers[args.arg_1] = registers["pc"] + 4;
        int shift = ((int)new_address - (int)registers["pc"]) / 4;
        jump(shift);
    }

    void beq(ThreeArgs& args) {
        if (registers[args.arg_1] == registers[args.arg_2]) {
            return jump(expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void bne(ThreeArgs& args) {
        if (registers[args.arg_1] != registers[args.arg_2]) {
            return jump(expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void blt(ThreeArgs& args) {
        if ((int)registers[args.arg_1] < (int)registers[args.arg_2]) {
            return jump(expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void bge(ThreeArgs& args) {
        if ((int)registers[args.arg_1] >= (int)registers[args.arg_2]) {
            return jump(expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void bltu(ThreeArgs& args) {
        if ((uint32_t)registers[args.arg_1] < (uint32_t)registers[args.arg_2]) {
            return jump(expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void bgeu(ThreeArgs& args) {
        if ((uint32_t)registers[args.arg_1] >= (uint32_t)registers[args.arg_2]) {
            return jump(expand_N_bit_value<13>(args.arg_3) / 4);
        }
        jump(1);
    }

    void lb(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = expand_N_bit_value<8>(cache.template read<1>(address));
        jump(1);
    }

    void lh(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = expand_N_bit_value<16>(cache.template read<2>(address));
        jump(1);
    }

    void lw(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.template read<4>(address);
        jump(1);
    }

    void lbu(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.template read<1>(address);
        jump(1);
    }

    void lhu(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.template read<2>(address);
        jump(1);
    }

    void sb(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        uint32_t byte_mask = (1 << 8) - 1;
        uint8_t byte = registers[args.arg_1] & byte_mask;
        cache.template write<1>(address, byte);
        jump(1);
    }

    void sh(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        uint32_t half_word_mask = (1 << 16) - 1;
        uint16_t half_word = registers[args.arg_1] & half_word_mask;
        cache.template write<2>(address, half_word);
        jump(1);
    }

    void sw(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        uint32_t word = registers[args.arg_1];
        cache.template write<4>(address, word);
        jump(1);
    }

    void addi(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] + expand_N_bit_value<12>(args.arg_3);
        jump(1);
    }

    void slti(ThreeArgs& args) {
        if ((int)registers[args.arg_2] < expand_N_bit_value<12>(args.arg_3)) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void sltiu(ThreeArgs& args) {
        if (registers[args.arg_2] < convert_to_unsigned_value(args.arg_3)) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void xori(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] ^ expand_N_bit_value<12>(args.arg_3);
        jump(1);
    }

    void ori(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] | expand_N_bit_value<12>(args.arg_3);
        jump(1);
    }

    void andi(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] & expand_N_bit_value<12>(args.arg_3);
        jump(1);
    }

    void slli(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] << convert_to_unsigned_value(args.arg_3);
        jump(1);
    }

    void srli(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] >> convert_to_unsigned_value(args.arg_3);
        jump(1);
    }

    void srai(ThreeArgs& args) {
        registers[args.arg_1] = (int)registers[args.arg_2] >> convert_to_unsigned_value(args.arg_3);
        jump(1);
    }

    void add(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] + registers[args.arg_3];
        jump(1);
    }

    void sub(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] - registers[args.arg_3];
        jump(1);
    }

    void sll(ThreeArgs& args) {
        uint32_t shift_mask = (1 << 5) - 1;
        uint32_t shift = registers[args.arg_3] & shift_mask;
        registers[args.arg_1] = registers[args.arg_2] << shift;
        jump(1);
    }

    void slt(ThreeArgs& args) {
        if ((int)registers[args.arg_2] < (int)registers[args.arg_3]) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void sltu(ThreeArgs& args) {
        if (registers[args.arg_2] < registers[args.arg_3]) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void xor_(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] ^ registers[args.arg_3];
        jump(1);
    }

    void srl(ThreeArgs& args) {
        uint32_t shift_mask = (1 << 5) - 1;
        uint32_t shift = registers[args.arg_3] & shift_mask;
        registers[args.arg_1] = registers[args.arg_2] >> shift;
        jump(1);
    }

    void sra(ThreeArgs& args) {
        uint32_t shift_mask = (1 << 5) - 1;
        uint32_t shift = registers[args.arg_3] & shift_mask;
        registers[args.arg_1] = (int)registers[args.arg_2] >> shift;
        jump(1);
    }

    void or_(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] | registers[args.arg_3];
        jump(1);
    }

    void and_(ThreeArgs& args) {
        registers[args.arg_1] = registers[args.arg_2] & registers[args.arg_3];
        jump(1);
    }

private:
    static uint32_t convert_to_unsigned_value(const std::string& value) {
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

    // переводит число записанное в N-битной форме, в 32-битную форму(с учётом знака)
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