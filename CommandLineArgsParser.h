#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

class CommandLineArgsParser {
public:
    CommandLineArgsParser(int argc, char* argv[]);

    const int& getReplacement() const;

    const std::string& getAsmFile() const;

    const std::string& getBinFile() const;

private:
    void parseArguments(int argc, char* argv[]);

    int replacement_ = -1;
    std::string asmFile_;
    std::string binFile_;
};
