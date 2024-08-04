#pragma once

#include "InstructionEncoder.h"
#include "AsmCommandArgs.h"
#include "ValueExpander.h"

#include <vector>
#include <iostream>


// todo add concepts
template <std::input_iterator InIt, std::output_iterator<char> OutIt>
class CodeEncoder {
public:
    explicit CodeEncoder(InIt commands_it_beg, InIt commands_it_end, OutIt out_it)
            : cur_it(commands_it_beg)
            , end_it(commands_it_beg)
            , out_it(out_it) {

    }

    void encode_all_commands() {
        while (cur_it != end_it) {
            uint32_t code;
            auto& command = *cur_it;
            if (two_args_instructions.contains(command[0])) {
                TwoArgsCommandArgs args{command[1], command[2]};
                code = two_args_instructions.at(command[0])(args);
            } else if (three_args_instructions.contains(command[0])) {
                ThreeArgsCommandArgs args{{command[1], command[2]}, command[3]};
                code = three_args_instructions.at(command[0])(args);
            } else {
                std::cerr << "Error: command " << command[0] << " not found" << std::endl;
                exit(-1);
            }
            *out_it = code;
            ++out_it;
        }
    }
private:
    static uint32_t lui(TwoArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::convert_to_unsigned_value(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(20, imm)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110111)
            .get_code();
        return code;
    }

    static uint32_t auipc(TwoArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::convert_to_unsigned_value(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(20, imm)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010111)
            .get_code();
        return code;
    }

    static uint32_t jal(TwoArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<21>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(1, slice(imm, 20, 20))
            .add_bits(10, slice(imm, 10, 1))
            .add_bits(1, slice(imm, 11, 11))
            .add_bits(8, slice(imm, 19, 12))
            .add_register(args.arg_1)
            .add_bits(7, 0b1101111)
            .get_code();
        return code;
    }

    static uint32_t mul(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000001)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b000)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t mulh(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000001)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b001)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t mulhsu(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000001)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b010)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t mulhu(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000001)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b011)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t div(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000001)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b100)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t divu(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000001)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b101)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t rem(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000001)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b110)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t remu(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000001)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b111)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t jalr(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b000)
            .add_register(args.arg_1)
            .add_bits(7, 0b1100111)
            .get_code();
        return code;
    }

    static uint32_t beq(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<13>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(1, slice(imm, 12, 12))
            .add_bits(6, slice(imm, 10, 5))
            .add_register(args.arg_2)
            .add_register(args.arg_1)
            .add_bits(3, 0b000)
            .add_bits(4, slice(imm, 4, 1))
            .add_bits(1, slice(imm, 11, 11))
            .add_bits(7, 0b1100011)
            .get_code();
        return code;
    }

    static uint32_t bne(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<13>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(1, slice(imm, 12, 12))
            .add_bits(6, slice(imm, 10, 5))
            .add_register(args.arg_2)
            .add_register(args.arg_1)
            .add_bits(3, 0b001)
            .add_bits(4, slice(imm, 4, 1))
            .add_bits(1, slice(imm, 11, 11))
            .add_bits(7, 0b1100011)
            .get_code();
        return code;

    }

    static uint32_t blt(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<13>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(1, slice(imm, 12, 12))
            .add_bits(6, slice(imm, 10, 5))
            .add_register(args.arg_2)
            .add_register(args.arg_1)
            .add_bits(3, 0b100)
            .add_bits(4, slice(imm, 4, 1))
            .add_bits(1, slice(imm, 11, 11))
            .add_bits(7, 0b1100011)
            .get_code();
        return code;
    }

    static uint32_t bge(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<13>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(1, slice(imm, 12, 12))
            .add_bits(6, slice(imm, 10, 5))
            .add_register(args.arg_2)
            .add_register(args.arg_1)
            .add_bits(3, 0b101)
            .add_bits(4, slice(imm, 4, 1))
            .add_bits(1, slice(imm, 11, 11))
            .add_bits(7, 0b1100011)
            .get_code();
        return code;
    }

    static uint32_t bltu(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<13>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(1, slice(imm, 12, 12))
            .add_bits(6, slice(imm, 10, 5))
            .add_register(args.arg_2)
            .add_register(args.arg_1)
            .add_bits(3, 0b110)
            .add_bits(4, slice(imm, 4, 1))
            .add_bits(1, slice(imm, 11, 11))
            .add_bits(7, 0b1100011)
            .get_code();
        return code;
    }

    static uint32_t bgeu(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<13>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(1, slice(imm, 12, 12))
            .add_bits(6, slice(imm, 10, 5))
            .add_register(args.arg_2)
            .add_register(args.arg_1)
            .add_bits(3, 0b111)
            .add_bits(4, slice(imm, 4, 1))
            .add_bits(1, slice(imm, 11, 11))
            .add_bits(7, 0b1100011)
            .get_code();
        return code;
    }

    static uint32_t lb(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_3)
            .add_bits(3, 0b000)
            .add_register(args.arg_1)
            .add_bits(7, 0b0000011)
            .get_code();
        return code;
    }

    static uint32_t lh(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_3)
            .add_bits(3, 0b001)
            .add_register(args.arg_1)
            .add_bits(7, 0b0000011)
            .get_code();
        return code;
    }

    static uint32_t lw(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_3)
            .add_bits(3, 0b010)
            .add_register(args.arg_1)
            .add_bits(7, 0b0000011)
            .get_code();
        return code;
    }

    static uint32_t lbu(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_3)
            .add_bits(3, 0b100)
            .add_register(args.arg_1)
            .add_bits(7, 0b0000011)
            .get_code();
        return code;    }

    static uint32_t lhu(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_3)
            .add_bits(3, 0b101)
            .add_register(args.arg_1)
            .add_bits(7, 0b0000011)
            .get_code();
        return code;
    }

    static uint32_t sb(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(7, slice(imm, 11, 5))
            .add_register(args.arg_1)
            .add_register(args.arg_3)
            .add_bits(3, 0b000)
            .add_bits(5, slice(imm, 4, 0))
            .add_bits(7, 0b0100011)
            .get_code();
        return code;
    }

    static uint32_t sh(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(7, slice(imm, 11, 5))
            .add_register(args.arg_1)
            .add_register(args.arg_3)
            .add_bits(3, 0b001)
            .add_bits(5, slice(imm, 4, 0))
            .add_bits(7, 0b0100011)
            .get_code();
        return code;
    }

    static uint32_t sw(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_2);
        uint32_t code = CodeEncoder()
            .add_bits(7, slice(imm, 11, 5))
            .add_register(args.arg_1)
            .add_register(args.arg_3)
            .add_bits(3, 0b010)
            .add_bits(5, slice(imm, 4, 0))
            .add_bits(7, 0b0100011)
            .get_code();
        return code;
    }

    static uint32_t addi(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b000)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t slti(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b010)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t sltiu(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b011)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t xori(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b100)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t ori(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b110)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t andi(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::expand_N_bit_value<12>(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b111)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t slli(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::convert_to_unsigned_value(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b001)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t srli(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::convert_to_unsigned_value(args.arg_3);
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b101)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t srai(ThreeArgsCommandArgs& args) {
        uint32_t imm = ValueExpander::convert_to_unsigned_value(args.arg_3);
        imm |= 1 << 10;
        uint32_t code = CodeEncoder()
            .add_bits(12, imm)
            .add_register(args.arg_2)
            .add_bits(3, 0b101)
            .add_register(args.arg_1)
            .add_bits(7, 0b0010011)
            .get_code();
        return code;
    }

    static uint32_t add(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b000)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t sub(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0100000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b000)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t sll(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b001)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t slt(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b010)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t sltu(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b011)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t xor_(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b100)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t srl(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b101)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t sra(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0100000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b101)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t or_(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b110)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011)
            .get_code();
        return code;
    }

    static uint32_t and_(ThreeArgsCommandArgs& args) {
        uint32_t code = CodeEncoder()
            .add_bits(7, 0b0000000)
            .add_register(args.arg_3)
            .add_register(args.arg_2)
            .add_bits(3, 0b111)
            .add_register(args.arg_1)
            .add_bits(7, 0b0110011
            ).get_code();
        return code;
    }
private:
    static uint32_t slice(uint32_t value, uint8_t first, uint8_t last) {
        value >>= last;
        value &= (1 << (first - last + 1)) - 1;
        return value;
    }
public:
    using ThreeArgsInstruction = uint32_t (*)(ThreeArgsCommandArgs&);
    inline static const std::unordered_map<std::string, ThreeArgsInstruction> three_args_instructions = {
            {"jalr",  &CodeEncoder::jalr}, {"beq", &CodeEncoder::beq}, {"bne", &CodeEncoder::bne},
            {"blt",   &CodeEncoder::blt}, {"bge", &CodeEncoder::bge}, {"bltu", &CodeEncoder::bltu},
            {"bgeu",  &CodeEncoder::bgeu}, {"lb", &CodeEncoder::lb}, {"lh", &CodeEncoder::lh}, {"lw", &CodeEncoder::lw},
            {"lbu",   &CodeEncoder::lbu}, {"lhu", &CodeEncoder::lhu}, {"sb", &CodeEncoder::sb}, {"sh", &CodeEncoder::sh},
            {"sw",    &CodeEncoder::sw}, {"addi", &CodeEncoder::addi}, {"slti", &CodeEncoder::slti},
            {"sltiu", &CodeEncoder::sltiu}, {"xori", &CodeEncoder::xori}, {"ori", &CodeEncoder::ori},
            {"andi",  &CodeEncoder::andi}, {"slli", &CodeEncoder::slli}, {"srli", &CodeEncoder::srli},
            {"srai",  &CodeEncoder::srai}, {"add", &CodeEncoder::add}, {"sub", &CodeEncoder::sub},
            {"sll",   &CodeEncoder::sll}, {"srl", &CodeEncoder::srl}, {"sra", &CodeEncoder::sra},
            {"or",    &CodeEncoder::or_}, {"and", &CodeEncoder::and_}, {"mul", &CodeEncoder::mul},
            {"mulh",  &CodeEncoder::mulh}, {"mulhsu", &CodeEncoder::mulhsu}, {"mulhu", &CodeEncoder::mulhu},
            {"div",   &CodeEncoder::div}, {"divu", &CodeEncoder::divu}, {"rem", &CodeEncoder::rem},
            {"remu",  &CodeEncoder::remu}, {"slt", &CodeEncoder::slt}, {"sltu", &CodeEncoder::sltu},
            {"xor",   &CodeEncoder::xor_}
    };
    using TwoArgsInstruction = uint32_t (*)(TwoArgsCommandArgs&);
    inline static const std::unordered_map<std::string, TwoArgsInstruction> two_args_instructions = {
            {"lui", &CodeEncoder::lui}, {"auipc", &CodeEncoder::auipc}, {"auipc", &CodeEncoder::auipc},
            {"jal", &CodeEncoder::jal}
    };
    InIt cur_it;
    InIt end_it;
    OutIt out_it;
};
