#include "CommandLineArgsParser.h"


CommandLineArgsParser::CommandLineArgsParser(int argc, char** argv) {
    parseArguments(argc, argv);
    if (replacement_ == -1) {
        std::cerr << "Error: --replacement argument is required." << std::endl;
        exit(-1);
    }
    if (asmFile_.empty()) {
        std::cerr << "Error: --asm argument is required." << std::endl;
        exit(-1);
    }
    if (binFile_.empty()) {
        std::cerr << "Error: --bin argument is required." << std::endl;
        exit(-1);
    }
}

const int& CommandLineArgsParser::getReplacement() const {
    return replacement_;
}

const std::string& CommandLineArgsParser::getAsmFile() const {
    return asmFile_;
}

const std::string& CommandLineArgsParser::getBinFile() const {
    return binFile_;
}

void CommandLineArgsParser::parseArguments(int argc, char** argv) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "--replacement") == 0) {
            char* pend = nullptr;
            replacement_ = std::strtoll(argv[++i], &pend, 10);
            if (*pend != '\0' || replacement_ < 0 || replacement_ > 2) {
                std::cerr << "Invalid value for --replacement. Must be 0, 1, or 2." << std::endl;
                exit(-1);
            }
            continue;
        }
        if (std::strcmp(argv[i], "--asm") == 0) {
            asmFile_ = argv[++i];
            continue;
        }
        if (std::strcmp(argv[i], "--bin") == 0) {
            binFile_ = argv[++i];
            continue;
        }
        std::cerr << "Unknown argument or missing value: " << argv[i] << std::endl;
    }
}



