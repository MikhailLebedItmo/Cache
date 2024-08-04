#include "CommandLineArgsParser.h"


CommandLineArgsParser::CommandLineArgsParser(int argc, char** argv) {
    parse_arguments(argc, argv);
    if (replacement_ == -1) {
        std::cerr << "Error: --replacement argument is required." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (asm_file_path.empty()) {
        std::cerr << "Error: --asm argument is required." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (bin_file_path.empty()) {
        std::cerr << "Error: --bin argument is required." << std::endl;
        exit(EXIT_FAILURE);
    }
}

const int& CommandLineArgsParser::get_replacement() const {
    return replacement_;
}

const std::string& CommandLineArgsParser::get_asm_file_path() const {
    return asm_file_path;
}

const std::string& CommandLineArgsParser::get_bin_file_path() const {
    return bin_file_path;
}

void CommandLineArgsParser::parse_arguments(int argc, char** argv) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "--replacement") == 0) {
            char* pend = nullptr;
            replacement_ = std::strtoll(argv[++i], &pend, 10);
            if (*pend != '\0' || replacement_ < 0 || replacement_ > 2) {
                std::cerr << "Invalid value for --replacement. Must be 0, 1, or 2." << std::endl;
                exit(EXIT_FAILURE);
            }
            continue;
        }
        if (std::strcmp(argv[i], "--asm") == 0) {
            asm_file_path = argv[++i];
            continue;
        }
        if (std::strcmp(argv[i], "--bin") == 0) {
            bin_file_path = argv[++i];
            continue;
        }
        std::cerr << "Unknown argument or missing value: " << argv[i] << std::endl;
    }
}



