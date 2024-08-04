#include "CommandLineArgsParser.h"
#include "AsmCommandParser.h"
#include "LRUCache.h"
#include "PLRUCache.h"
#include "Interpreter.h"

#include <vector>
#include <string>
#include <fstream>

const int MEM_SIZE = 256 * (1 << 10);
const int CACHE_SIZE = 4 * (1 << 10);
const int CACHE_LINE_SIZE = 32;
const int CACHE_LINE_COUNT = 128;
const int CACHE_WAY = 4;
const int CACHE_SETS = 32;
const int ADDR_LEN = 18;
const int CACHE_TAG_LEN = 8;
const int CACHE_INDEX_LEN = 5;
const int CACHE_OFFSET_LEN = 5;

int main(int argc, char** argv) {
    CommandLineArgsParser cmd_args_parser(argc, argv);
    std::ifstream asm_file(cmd_args_parser.get_asm_file_path());
    if (!asm_file.is_open()) {
        std::cerr << "failed to open " << cmd_args_parser.get_asm_file_path() << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<std::vector<std::string>> commands;
    AsmCommandParser asm_command_parser(
            std::istreambuf_iterator<char>(asm_file)
            , std::istreambuf_iterator<char>()
            , std::back_inserter(commands)
    );
    int replacement = cmd_args_parser.get_replacement();
    if (replacement == 0 || replacement == 1) {
        LRUCache<CACHE_WAY, CACHE_TAG_LEN, CACHE_INDEX_LEN, CACHE_OFFSET_LEN> cache;
        Interpreter interpreter(commands, cache);
        auto [hits_cnt, requests_cnt] = interpreter.run();
        double hits_percent = (double)hits_cnt / requests_cnt * 100;
        std::printf("LRU\thit rate: %3.4f%%\n", hits_percent);
    }
    if (replacement == 0 || replacement == 2) {
        PLRUCache<CACHE_WAY, CACHE_TAG_LEN, CACHE_INDEX_LEN, CACHE_OFFSET_LEN> cache;
        Interpreter interpreter(commands, cache);
        auto [hits_cnt, requests_cnt] = interpreter.run();
        double hits_percent = (double)hits_cnt / requests_cnt * 100;
        std::printf("pLRU\thit rate: %3.4f%%\n", hits_percent);
    }
    return 0;
}
