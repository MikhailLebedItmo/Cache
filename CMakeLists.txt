cmake_minimum_required(VERSION 3.26)
project(se_comp_arch24_cache_isa_MikhailLebedItmo)

set(CMAKE_CXX_STANDARD 20)

add_executable(se_comp_arch24_cache_isa_MikhailLebedItmo
        main.cpp
        AddressConfig.h
        MEMLine.h
        MEM.h
        LRUCacheLine.h
        LRUCacheLines.h
        LRUCache.h
        PLRUCacheLine.h
        PLRUCacheLines.h
        PLRUCache.h
        ICahe.h
        AsmCommandParser.h
        Registers.h
        Registers.cpp
        Interpreter.h
        CommandLineArgsParser.h
        CommandLineArgsParser.cpp
        CacheLookupResult.h
        CodeEncoder.h
        InstructionEncoder.h
        InstructionEncoder.cpp
        AsmCommandArgs.h
        ValueExpander.h
        ValueExpander.cpp
)

