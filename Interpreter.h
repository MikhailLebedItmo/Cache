#pragma once

#include "ICahe.h"
#include "Registers.h"

#include <unordered_map>
#include <vector>
#include <iostream>


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
    using Commands = std::vector<std::vector<std::string>>;
private:
    using TwoArgsInstruction = void (Interpreter::*)(TwoArgs);
    using ThreeArgsInstruction = void (Interpreter::*)(ThreeArgs);
public:
    Interpreter(Commands code, ICache<Implementation>& cache)
        : code(std::move(code))
        , cur_command(this->code.begin())
        , registers()
        , cache(cache) {

    }

    std::pair<uint32_t, uint32_t> run() {
        while (cur_command->front() != "jalr" || ((*cur_command)[2] != "ra" && (*cur_command)[2] != "x1")) {
            if (cur_command->size() == 3) {
                TwoArgs args = {(*cur_command)[1], (*cur_command)[2]};
                two_args_instructions.at(cur_command->front())(args);
            } else if (cur_command->size() == 4) {
                ThreeArgs args{
                        {(*cur_command)[1], (*cur_command)[2]}, (*cur_command)[3]
                };
                three_args_instructions.at(cur_command->front())(args);
            } else {
                std::cerr << "Error: wrong command";
            }
        }

        return {cache.get_hit_cnt(), cache.get_request_cnt()};
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
        registers[args.arg_1] = res >> 32; // todo maby int>>shift
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
        registers[args.arg_1] = expand_signed_byte(cache.read_1_byte(address));
        jump(1);
    }

    void lh(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = expand_2_signed_byte(cache.read_2_bytes(address));
        jump(1);
    }

    void lw(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.read_4_bytes(address);
        jump(1);
    }

    void lbu(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.read_1_byte(address);
        jump(1);
    }

    void lhu(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        registers[args.arg_1] = cache.read_2_bytes(address);
        jump(1);
    }

    void sb(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        uint32_t byte_mask = (1 << 8) - 1;
        uint8_t byte = registers[args.arg_1] & byte_mask;
        cache.write_1_byte(address, byte);
        jump(1);
    }

    void sh(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        uint32_t half_word_mask = (1 << 16) - 1;
        uint16_t half_word = registers[args.arg_1] & half_word_mask;
        cache.write_2_bytes(address, half_word);
        jump(1);
    }

    void sw(ThreeArgs& args) {
        uint32_t address = registers[args.arg_3] + expand_N_bit_value<12>(args.arg_2);
        uint32_t word = registers[args.arg_1];
        cache.write_4_bytes(address, word);
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

    // todo check signed and unsigned <
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

    void slt(ThreeArgs& args) { // todo: add to map
        if ((int)registers[args.arg_2] < (int)registers[args.arg_3]) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void sltu(ThreeArgs& args) { // todo: add to map
        if (registers[args.arg_2] < registers[args.arg_3]) {
            registers[args.arg_1] = 1;
        } else {
            registers[args.arg_1] = 0;
        }
        jump(1);
    }

    void xor_(ThreeArgs& args) { // todo: add to map
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
        if (value.size() > 2 && value[1] == 'x') {
            // todo: maby bad input
            ivalue = std::stoul(value, nullptr, 16);
        } else {
            ivalue = std::stoul(value, nullptr, 10);
        }
        return ivalue;
    }
    template <uint8_t N> // todo: N < 32
    static int32_t expand_N_bit_value(const std::string& value) {
        uint32_t ivalue = convert_to_unsigned_value(value);
        if ((ivalue >> (N - 1)) & 1) {
            // number is negative => add 1 to the beginning
            uint32_t mask = (1 << (32 - N)) - 1;
            mask <<= N;
            ivalue |= mask;
        }
        return static_cast<int32_t>(ivalue);
    }

    void jump(int shift) {
        cur_command += shift;
        registers["pc"] += 4 * shift;
    }
private:
    inline static const std::unordered_map<std::string, TwoArgsInstruction> two_args_instructions = {
            {"lui", &Interpreter::lui}, {"auipc", &Interpreter::auipc}, {"auipc", &Interpreter::auipc},
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
    Commands code;
    Commands::iterator cur_command;
    Registers registers;
    ICache<Implementation>& cache;
};