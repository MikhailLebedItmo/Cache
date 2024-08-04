// Класс для парсинга ассемблерных команд из файла с кодом, в вектор строк

// Все комманды в файле должны быть записаны в формате:
// Инструкции с 3 аргументами "%-7s\t%s, %s, %s\n"
// Инструкции с 2 аргументами "%-7s\t%s, %s\n"

// Для большей вариативности класс конструируется из input-итераторов на начало и конец потока с кодом,
// и output-итератора на поток - куда требуется записать распарсенные команды.

#pragma once

#include <unordered_set>
#include <string>
#include <istream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdint>
#include <iostream>

template <std::input_iterator InIt, std::output_iterator<std::vector<std::string>> OutIt>
requires requires (InIt it) {{*it} -> std::convertible_to<char>;}
class AsmCommandParser {
private:
    inline static const std::unordered_set<std::string> two_args_commands = {"lui", "auipc", "jal"};
    inline static const std::unordered_set<std::string> three_args_commands = {
            "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu", "lb", "lh", "lw", "lbu", "lhu", "sb", "sh", "sw",
            "addi", "slti", "sltiu", "xori", "ori", "andi", "slli", "srli", "srai", "add", "sub", "sll", "srl",
            "sra", "or", "and", "mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu", "slt", "sltu", "xor"
    };
public:
    AsmCommandParser(InIt code_beg, InIt code_end, OutIt out)
        : cur(code_beg), end(code_end), out(out) {
        parse_all_commands();
    }
private:
    void parse_all_commands() {
        while (cur != end) {
            parse_command();
        }
    }

    void parse_command() {
        std::vector<std::string> command = {parse_command_name()};
        const std::string& name = command[0];
        if (two_args_commands.contains(name)) {
            parse_args(command, 2);
        } else if (three_args_commands.contains(name)) {
            parse_args(command, 3);
        } else {
            std::cerr << "Error: command " << name << " not found" << std::endl;
            exit(EXIT_FAILURE);
        }
        *out = std::move(command);
        ++out;
    }

    std::string parse_command_name() {
        std::string command;
        while (*cur != '\t') {
            if (*cur != ' ') {
                command.push_back(*cur);
                ++cur;
            }
        }
        ++cur;  // skip "\t"

        return command;
    }

    void parse_args(std::vector<std::string>& args_out, uint32_t args_cnt) {
        if (args_cnt == 0) return;
        std::string arg;
        for (uint32_t i = 0; i < args_cnt - 1; ++i) {
            while (*cur != ',') {
                arg.push_back(*cur);
                ++cur;
            }
            ++++cur;  // skip ", "
            args_out.push_back(std::move(arg));
        }
        while (*cur != '\n') {
            arg.push_back(*cur);
            ++cur;
        }
        ++cur; // skip "\n"
        args_out.push_back(std::move(arg));
    }
private:
    InIt cur;
    InIt end;
    OutIt out;
};