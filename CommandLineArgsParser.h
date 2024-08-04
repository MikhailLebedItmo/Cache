// Класс для парсинга аргументов командной строки

// --replacement <политика_вытеснения> 0, 1 или 2
// --asm <имя_файла_с_кодом_ассемблера>
// --bin <имя_файла_с_машинным_кодом>

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

class CommandLineArgsParser {
public:
    CommandLineArgsParser(int argc, char* argv[]);

    const int& get_replacement() const;

    const std::string& get_asm_file_path() const;

    const std::string& get_bin_file_path() const;

private:
    void parse_arguments(int argc, char* argv[]);

    int replacement_ = -1;
    std::string asm_file_path;
    std::string bin_file_path;
};
