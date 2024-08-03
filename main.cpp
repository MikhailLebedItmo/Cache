//#include <iostream>
//#include <cstdint>
//#include <utility>
//#include <vector>
//#include <tuple>
//#include <string>
//#include <unordered_set>
//#include <unordered_map>
//#include <algorithm>
//#include <bitset>
//#include <optional>
//#include <filesystem>
//#include <fstream>
//
//
//
//const int MEM_SIZE = 256 * (1 << 10);
//const int CACHE_SIZE = 4 * (1 << 10);
//const int CACHE_LINE_SIZE = 32;
//const int CACHE_LINE_COUNT = 128;
//const int CACHE_WAY = 4;
//const int CACHE_SETS = 32;
//const int ADDR_LEN = 18;
//const int CACHE_TAG_LEN = 8;
//const int CACHE_INDEX_LEN = 5;
//const int CACHE_OFFSET_LEN = 5;
//
//class AddressConfig {
//protected:
//    const uint8_t tag_len;
//    const uint8_t index_len;
//    const uint8_t offset_len;
//
//    AddressConfig(uint8_t tag_len, uint8_t index_len, uint8_t offset_len)
//            : tag_len(tag_len)
//            , index_len(index_len)
//            , offset_len(offset_len) {
//
//    }
//
//    std::tuple<uint32_t, uint32_t, uint32_t> split_address(uint32_t address) const {
//        uint32_t tag = address >> (index_len + offset_len);
//        uint32_t index_mask = ((uint32_t)1 << index_len) - 1;
//        uint32_t index = (address >> offset_len) & index_mask;
//        uint32_t offset_mask = ((uint32_t)1 << offset_len) - 1;
//        uint32_t offset = address & offset_mask;
//
//        return {tag, index, offset};
//    }
//};
//
//
//struct MEMLine {
//    std::vector<uint8_t> bytes;
//    uint32_t tag = 2000000000;
//};
//
//class MEM : private AddressConfig {
//public:
//    MEM(uint8_t tag_len, uint8_t index_len, uint8_t offset_len)
//            : AddressConfig(tag_len, index_len, offset_len)
//            , bytes((uint32_t)1 << (tag_len + index_len + offset_len)) {
//
//    }
//
//    MEMLine read_line(uint32_t address) {
//
//        auto [tag, index, offset] = split_address(address);
//        uint32_t first_cell_index = address >> offset_len << offset_len;
//        uint32_t last_cell_index = first_cell_index + (1 << offset_len);
//        return {std::vector<uint8_t>(bytes.begin() + first_cell_index, bytes.begin() + last_cell_index), tag};
//    }
//
//    void write_line(const MEMLine& mem_line, uint32_t line_index) {
//        uint32_t block_size = (uint32_t)1 << (index_len + offset_len);
//        uint32_t line_size = (uint32_t)1 << (offset_len);
//        uint32_t first_cell_index = mem_line.tag * block_size + line_index * line_size;
//        for (auto byte: mem_line.bytes) {
//            bytes[first_cell_index++] = byte;
//        }
//    }
//private:
//    std::vector<uint8_t> bytes;
//};
//
//struct LRUCacheLine : public MEMLine {
//    uint32_t position = 0;
//
//    LRUCacheLine(uint32_t line_size): MEMLine{std::vector<uint8_t>(line_size)} {
//
//    }
//
//    LRUCacheLine(MEMLine mem_line): MEMLine(std::move(mem_line)) {
//
//    }
//};
//
//struct PLRUCacheLine: public MEMLine {
//    bool flag = false;
//
//    PLRUCacheLine(uint32_t line_size): MEMLine{std::vector<uint8_t>(line_size)} {
//
//    }
//
//    PLRUCacheLine(MEMLine mem_line): MEMLine(std::move(mem_line)) {
//
//    }
//};
//
//class PLRUCacheWay : private AddressConfig {
//public:
//    PLRUCacheWay(uint8_t tag_len, uint8_t index_len, uint8_t offset_len)
//            : AddressConfig(tag_len, index_len, offset_len)
//            , lines((uint32_t)1 << index_len, PLRUCacheLine((uint32_t)1 << offset_len)){
//
//    }
//
//    bool is_contains(uint32_t address) const {
//        auto [tag, index, offset] = split_address(address);
//        return lines[index].tag == tag;
//    }
//
//    uint8_t read_mem_cell(uint32_t address) {
//        return get_mem_cell(address);
//    }
//
//    void write_mem_cell(uint32_t address, uint8_t value) {
//        get_mem_cell(address) = value;
//    }
//
//    void update_line(MEM& memory, uint32_t address) {
//        auto [tag, index, offset] = split_address(address);
//        if (lines[index].tag < ((uint32_t)1 << tag_len)) {
//            memory.write_line(lines[index], index);
//        }
//
//        PLRUCacheLine new_line(memory.read_line(address));
//        new_line.flag = true;
//        lines[index] = std::move(new_line);
//    }
//
//    bool get_flag(uint32_t address) {
//        auto [tag, index, offset] = split_address(address);
//        return lines[index].flag;
//    }
//
//    void update_flag(uint32_t address, bool new_flag) {
//        auto [tag, index, offset] = split_address(address);
//        lines[index].flag = new_flag;
//    }
//private:
//    std::vector<PLRUCacheLine> lines;
//
//    uint8_t& get_mem_cell(uint32_t address) {
//        if (!is_contains(address)) {
//            std::cerr << "Error: mem_t not find" << std::endl;
//            exit(-1);
//        }
//        auto [tag, index, offset] = split_address(address);
//        return lines[index].bytes[offset];
//    }
//};
//
//class LRUCacheWay : private AddressConfig {
//public:
//    LRUCacheWay(uint32_t start_pos, uint8_t tag_len, uint8_t index_len, uint8_t offset_len)
//            : AddressConfig(tag_len, index_len, offset_len)
//            , lines((uint32_t)1 << index_len, LRUCacheLine((uint32_t)1 << offset_len)){
//        for (auto& line: lines) {
//            line.position = start_pos;
//        }
//    }
//
//    bool is_contains(uint32_t address) const {
//        auto [tag, index, offset] = split_address(address);
//        return lines[index].tag == tag;
//    }
//
//    uint8_t read_mem_cell(uint32_t address) {
//        return get_mem_cell(address);
//    }
//
//    void write_mem_cell(uint32_t address, uint8_t value) {
//        get_mem_cell(address) = value;
//    }
//
//    void update_line(MEM& memory, uint32_t address) {
//        auto [tag, index, offset] = split_address(address);
//        if (lines[index].tag < ((uint32_t)1 << tag_len)) {
//            memory.write_line(lines[index], index);
//        }
//
//        LRUCacheLine new_line(memory.read_line(address));
//        new_line.position = lines[index].position;
//        lines[index] = std::move(new_line);
//    }
//
//    uint32_t get_position(uint32_t address) {
//        auto [tag, index, offset] = split_address(address);
//        return lines[index].position;
//    }
//
//    void update_position(uint32_t address, uint32_t position) {
//        auto [tag, index, offset] = split_address(address);
//        if (lines[index].position == position) {
//            lines[index].position = 0;
//        } else if (lines[index].position < position) {
//            ++lines[index].position;
//        }
//    }
//private:
//    std::vector<LRUCacheLine> lines;
//
//    uint8_t& get_mem_cell(uint32_t address) {
//        if (!is_contains(address)) {
//            std::cerr << "Error: mem_t not find" << std::endl;
//            exit(-1);
//        }
//        auto [tag, index, offset] = split_address(address);
//        return lines[index].bytes[offset];
//    }
//};
//
//
//class Cache {
//protected:
//    uint32_t hit_cnt;
//    uint32_t request_cnt;
//    MEM memory;
//
//    Cache(uint8_t tag_len, uint8_t index_len, uint8_t offset_len)
//            : hit_cnt(0)
//            , request_cnt(0)
//            , memory(tag_len, index_len, offset_len) {
//
//    }
//public:
//    virtual uint8_t read_1_byte(uint32_t address) = 0;
//
//    virtual uint16_t read_2_bytes(uint32_t address) = 0;
//
//    virtual uint32_t read_4_bytes(uint32_t address) = 0;
//
//    virtual void write_1_byte(uint32_t address, uint8_t value) = 0;
//
//    virtual void write_2_bytes(uint32_t address, uint16_t value) = 0;
//
//    virtual void write_4_bytes(uint32_t address, uint32_t value) = 0;
//
//    uint32_t get_hit_cnt() {
//        return hit_cnt;
//    }
//
//    uint32_t get_request_cnt() {
//        return request_cnt;
//    }
//};
//
//class PLRUCache : public Cache {
//public:
//    PLRUCache(uint32_t block_cnt, uint8_t tag_len, uint8_t index_len, uint8_t offset_len)
//            : Cache(tag_len, index_len, offset_len)
//            , ways(block_cnt, PLRUCacheWay(tag_len, index_len, offset_len)) {
//
//    }
//
//    uint8_t read_1_byte(uint32_t address) override {
//        ++request_cnt;
//        auto [byte, is_not_hit] = read_mem_cell(address);
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//        return byte;
//    }
//
//    uint16_t read_2_bytes(uint32_t address) override {
//        ++request_cnt;
//        auto [byte_1, is_not_hit_1] = read_mem_cell(address++);
//        auto [byte_2, is_not_hit_2] = read_mem_cell(address);
//        bool is_not_hit = is_not_hit_1 || is_not_hit_2;
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//        return ((uint16_t)byte_1 << 8) + byte_2;
//    }
//
//    uint32_t read_4_bytes(uint32_t address) override {
//        ++request_cnt;
//        auto [byte_1, is_not_hit_1] = read_mem_cell(address++);
//        auto [byte_2, is_not_hit_2] = read_mem_cell(address++);
//        auto [byte_3, is_not_hit_3] = read_mem_cell(address++);
//        auto [byte_4, is_not_hit_4] = read_mem_cell(address);
//        bool is_not_hit = is_not_hit_1 || is_not_hit_2 || is_not_hit_3 || is_not_hit_4;
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//        uint32_t res = byte_1;
//        res = (res << 8) + byte_2;
//        res = (res << 8) + byte_3;
//        res = (res << 8) + byte_4;
//        return res;
//    }
//
//    void write_1_byte(uint32_t address, uint8_t value) override {
//        ++request_cnt;
//        bool is_not_hit = write_mem_cell(address, value);
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//    }
//
//    void write_2_bytes(uint32_t address, uint16_t value) override {
//        ++request_cnt;
//        uint16_t byte_mask = (1 << 8) - 1;
//        uint8_t byte_1 = value >> 8;
//        uint8_t byte_2 = value & byte_mask;
//        bool is_not_hit = write_mem_cell(address++, byte_1);
//        is_not_hit = write_mem_cell(address, byte_2) || is_not_hit;
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//    }
//
//    void write_4_bytes(uint32_t address, uint32_t value) override {
//        ++request_cnt;
//        uint32_t byte_mask = (1 << 8) - 1;
//        uint8_t byte_1 = value >> 24;
//        uint8_t byte_2 = (value >> 16) & byte_mask;
//        uint8_t byte_3 = (value >> 8) & byte_mask;
//        uint8_t byte_4 = value & byte_mask;
//        bool is_not_hit = write_mem_cell(address++, byte_1);
//        is_not_hit = write_mem_cell(address++, byte_2) || is_not_hit;
//        is_not_hit = write_mem_cell(address++, byte_3) || is_not_hit;
//        is_not_hit = write_mem_cell(address, byte_4) || is_not_hit;
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//    }
//
//private:
//    std::vector<PLRUCacheWay> ways;
//
//    std::pair<uint8_t, bool> read_mem_cell(uint32_t address) {
//        bool is_not_hit = update_line(address);
//        for (auto& way: ways) {
//            if (way.is_contains(address)) {
//                return {way.read_mem_cell(address), is_not_hit};
//            }
//        }
//        std::cerr << "Error: LRUCache read_1_byte cell not find" << std::endl;
//        exit(-1);
//    }
//
//    bool write_mem_cell(uint32_t address, uint8_t value) {
//        bool is_not_hit = update_line(address);
//        for (auto& way: ways) {
//            if (way.is_contains(address)) {
//                way.write_mem_cell(address, value);
//                return is_not_hit;
//            }
//        }
//        std::cerr << "Error: LRUCache write_mem_cell cell not find" << std::endl;
//        exit(-1);
//    }
//
//    std::pair<bool, uint32_t> is_contains(uint32_t address) {
//        for (int i = 0; i < ways.size(); ++i) {
//            if (ways[i].is_contains(address)) {
//                return {true, i};
//            }
//        }
//        return {false, -1};
//    }
//
//    bool update_line(uint32_t address) { // return is_not_hit
//        if (auto [contains, pos] = is_contains(address); contains) {
//            ways[pos].update_flag(address, true);
//            bool need_reset = true;
//            for (auto& way: ways) {
//                need_reset = need_reset && way.get_flag(address);
//            }
//            if (need_reset) {
//                for (auto& way: ways) {
//                    way.update_flag(address, false);
//                }
//                ways[pos].update_flag(address, true);
//            }
//            return false;
//        }
//        uint32_t del_ind = -1;
//        for (int i = 0; i < ways.size(); ++i) {
//            auto flag = ways[i].get_flag(address);
//            if (!flag) {
//                del_ind = i;
//                break;
//            }
//        }
//        if (del_ind == -1) {
//            for (auto& way: ways) {
//                way.update_flag(address, false);
//            }
//            del_ind = 0;
//        }
//        ways[del_ind].update_line(memory, address);
//
//        return true;
//    }
//};
//
//class LRUCache : public Cache {
//public:
//    LRUCache(uint32_t block_cnt, uint8_t tag_len, uint8_t index_len, uint8_t offset_len)
//            : Cache(tag_len, index_len, offset_len) {
//        ways.reserve(block_cnt);
//        for (int i = 0; i < block_cnt; ++i) {
//            ways.emplace_back(i, tag_len, index_len, offset_len);
//        }
//    }
//
//    uint8_t read_1_byte(uint32_t address) override {
//        ++request_cnt;
//        auto [byte, is_not_hit] = read_mem_cell(address);
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//        return byte;
//    }
//
//    uint16_t read_2_bytes(uint32_t address) override {
//        ++request_cnt;
//        auto [byte_1, is_not_hit_1] = read_mem_cell(address++);
//        auto [byte_2, is_not_hit_2] = read_mem_cell(address);
//        bool is_not_hit = is_not_hit_1 || is_not_hit_2;
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//        return ((uint16_t)byte_1 << 8) + byte_2;
//    }
//
//    uint32_t read_4_bytes(uint32_t address) override {
//        ++request_cnt;
//        auto [byte_1, is_not_hit_1] = read_mem_cell(address++);
//        auto [byte_2, is_not_hit_2] = read_mem_cell(address++);
//        auto [byte_3, is_not_hit_3] = read_mem_cell(address++);
//        auto [byte_4, is_not_hit_4] = read_mem_cell(address);
//        bool is_not_hit = is_not_hit_1 || is_not_hit_2 || is_not_hit_3 || is_not_hit_4;
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//        uint32_t res = byte_1;
//        res = (res << 8) + byte_2;
//        res = (res << 8) + byte_3;
//        res = (res << 8) + byte_4;
//        return res;
//    }
//
//    void write_1_byte(uint32_t address, uint8_t value) override {
//        ++request_cnt;
//        bool is_not_hit = write_mem_cell(address, value);
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//    }
//
//    void write_2_bytes(uint32_t address, uint16_t value) override {
//        ++request_cnt;
//        uint16_t byte_mask = (1 << 8) - 1;
//        uint8_t byte_1 = value >> 8;
//        uint8_t byte_2 = value & byte_mask;
//        bool is_not_hit = write_mem_cell(address++, byte_1);
//        is_not_hit = write_mem_cell(address, byte_2) || is_not_hit;
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//    }
//
//    void write_4_bytes(uint32_t address, uint32_t value) override {
//        ++request_cnt;
//        uint32_t byte_mask = (1 << 8) - 1;
//        uint8_t byte_1 = value >> 24;
//        uint8_t byte_2 = (value >> 16) & byte_mask;
//        uint8_t byte_3 = (value >> 8) & byte_mask;
//        uint8_t byte_4 = value & byte_mask;
//        bool is_not_hit = write_mem_cell(address++, byte_1);
//        is_not_hit = write_mem_cell(address++, byte_2) || is_not_hit;
//        is_not_hit = write_mem_cell(address++, byte_3) || is_not_hit;
//        is_not_hit = write_mem_cell(address, byte_4) || is_not_hit;
//        if (!is_not_hit) {
//            ++hit_cnt;
//        }
//    }
//private:
//    std::vector<LRUCacheWay> ways;
//
//    std::pair<uint8_t, bool> read_mem_cell(uint32_t address) {
//        bool is_not_hit = update_line(address);
//        for (auto& way: ways) {
//            if (way.is_contains(address)) {
//                return {way.read_mem_cell(address), is_not_hit};
//            }
//        }
//        std::cerr << "Error: LRUCache read_1_byte cell not find" << std::endl;
//        exit(-1);
//    }
//
//    bool write_mem_cell(uint32_t address, uint8_t value) {
//        bool is_not_hit = update_line(address);
//        for (auto& way: ways) {
//            if (way.is_contains(address)) {
//                way.write_mem_cell(address, value);
//                return is_not_hit;
//            }
//        }
//        std::cerr << "Error: LRUCache write_mem_cell cell not find" << std::endl;
//        exit(-1);
//    }
//
//    std::pair<bool, uint32_t> is_contains(uint32_t address) {
//        for (auto& way: ways) {
//            if (way.is_contains(address)) {
//                return {true, way.get_position(address)};
//            }
//        }
//
//        return {false, 0};
//    }
//
//    bool update_line(uint32_t address) {
//        if (auto [contains, pos] = is_contains(address); contains) {
//            for (auto& way: ways) {
//                way.update_position(address, pos);
//            }
//            return false;
//        }
//        size_t max_position_ind = 0;
//        uint32_t max_position = 0;
//
//        for (int i = 0; i < ways.size(); ++i) {
//            auto position = ways[i].get_position(address);
//            if (position > max_position) {
//                max_position_ind = i;
//                max_position = position;
//            }
//        }
//
//        ways[max_position_ind].update_line(memory, address);
//        for (auto& way: ways) {
//            way.update_position(address, max_position);
//        }
//
//        return true;
//    }
//};
//
//class AsmCommandParser {
//public:
//    static const std::unordered_set<std::string> two_args_commands;
//    static const std::unordered_set<std::string> three_args_commands;
//
//    explicit AsmCommandParser(const std::filesystem::path& file_path) {
//        std::ifstream file(file_path);
//        std::stringstream buffer;
//        buffer << file.rdbuf();
//        code = buffer.str();
//        cur_pos = this->code.begin();
//    }
//
//    void parse_all_commands() {
//        while (cur_pos != code.end()) {
//            parse_command();
//        }
//    }
//
//    std::vector<std::vector<std::string>>&& get_commands() {
//        return std::move(commands);
//    }
//private:
//    void lower(std::string& s) {
//        std::transform(s.begin(), s.end(), s.begin(),
//                       [](unsigned char c){ return std::tolower(c);});
//    }
//
//    std::pair<std::string, std::string> parse_two_args() {
//        std::pair<std::string, std::string> args;
//        while (*cur_pos != ',') {
//            args.first.push_back(*cur_pos);
//            ++cur_pos;
//        }
//        ++cur_pos;
//        ++cur_pos;
//        while (*cur_pos != '\n') {
//            args.second.push_back(*cur_pos);
//            ++cur_pos;
//        }
//        ++cur_pos;
//        lower(args.first);
//        lower(args.second);
//        return args;
//    }
//
//    std::tuple<std::string, std::string, std::string> parse_three_args() {
//        std::tuple<std::string, std::string, std::string> args;
//        while (*cur_pos != ',') {
//            std::get<0>(args).push_back(*cur_pos);
//            ++cur_pos;
//        }
//        ++cur_pos;
//        ++cur_pos;
//        while (*cur_pos != ',') {
//            std::get<1>(args).push_back(*cur_pos);
//            ++cur_pos;
//        }
//        ++cur_pos;
//        ++cur_pos;
//        while (*cur_pos != '\n') {
//            std::get<2>(args).push_back(*cur_pos);
//            ++cur_pos;
//        }
//        lower(std::get<0>(args));
//        lower(std::get<2>(args));
//        lower(std::get<2>(args));
//        ++cur_pos;
//
//        return args;
//    }
//
//    void parse_command() {
//        std::string command;
//        while (*cur_pos != '\t') {
//            if (*cur_pos != ' ') {
//                command.push_back(*cur_pos);
//                ++cur_pos;
//            }
//        }
//        lower(command);
//        ++cur_pos;
//        if (two_args_commands.contains(command)) {
//            auto [arg_1, arg_2] = parse_two_args();
//            commands.emplace_back();
//            commands.back().push_back(std::move(command));
//            commands.back().push_back(std::move(arg_1));
//            commands.back().push_back(std::move(arg_2));
//            return;
//        }
//        if (three_args_commands.contains(command)) {
//            auto [arg_1, arg_2, arg_3] = parse_three_args();
//            commands.emplace_back();
//            commands.back().push_back(std::move(command));
//            commands.back().push_back(std::move(arg_1));
//            commands.back().push_back(std::move(arg_2));
//            commands.back().push_back(std::move(arg_3));
//            return;
//        }
//        std::cerr << "Error: command " << command << " not found" << std::endl;
//        exit(-1);
//    }
//
//private:
//    std::string code;
//    std::string::iterator cur_pos;
//    std::vector<std::vector<std::string>> commands;
//};
//
//const std::unordered_set<std::string> AsmCommandParser::two_args_commands = {"lui", "auipc", "jal"};
//const std::unordered_set<std::string> AsmCommandParser::three_args_commands = {
//        "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu", "lb", "lh", "lw", "lbu", "lhu", "sb", "sh", "sw",
//        "addi", "slti", "sltiu", "xori", "ori", "andi", "slli", "srli", "srai", "add", "sub", "sll", "srl",
//        "sra", "or", "and", "mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu", "slt", "sltu", "xor"
//};
//
//class Registers {
//public:
//    static const std::vector<std::string> registers_names_1;
//    static const std::vector<std::string> registers_names_2;
//
//    Registers() {
//        values.resize(registers_names_1.size());
//        for (int i = 0; i < registers_names_1.size(); ++i) {
//            registers[registers_names_1[i]] = values.begin() + i;
//        }
//        for (int i = 0; i < registers_names_2.size(); ++i) {
//            registers[registers_names_2[i]] = values.begin() + i;
//        }
//        registers["fp"] = values.begin() + 8;
//    }
//
//    uint32_t& operator[] (const std::string& key) {
//        *registers["zero"] = 0;
//        return *registers[key];
//    }
//
//    bool contains(const std::string& key) {
//        return registers.contains(key);
//    }
//private:
//    std::vector<uint32_t> values;
//    std::unordered_map<std::string, std::vector<uint32_t>::iterator> registers;
//};
//
//const std::vector<std::string> Registers::registers_names_1 = {
//        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
//        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6", "pc"
//};
//
//const std::vector<std::string> Registers::registers_names_2 = {
//        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16",
//        "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
//};
//
//struct InterpreterTwoArgsCommandArgs {
//    std::vector<std::vector<std::string>>::iterator& cur_command;
//    Registers& registers;
//    Cache& cache;
//    std::string& arg_1;
//    std::string& arg_2;
//};
//
//struct InterpreterThreeArgsCommandArgs : public InterpreterTwoArgsCommandArgs {
//    std::string& arg_3;
//};
//
//class Interpreter {
//public:
//    Interpreter(std::vector<std::vector<std::string>> commands, Cache& cache)
//            : commands(std::move(commands))
//            , cur_command(this->commands.begin())
//            , registers()
//            , cache(cache) {}
//
//    std::pair<uint32_t, uint32_t> run() {
//        while (cur_command->front() != "jalr" || ((*cur_command)[2] != "ra" && (*cur_command)[2] != "x1")) {
//            if (cur_command->size() == 3) {
//                InterpreterTwoArgsCommandArgs args = {cur_command, registers, cache, (*cur_command)[1], (*cur_command)[2]};
//                two_args_instructions.at(cur_command->front())(args);
//            } else if (cur_command->size() == 4) {
//                InterpreterThreeArgsCommandArgs args{
//                        {cur_command, registers, cache, (*cur_command)[1], (*cur_command)[2]}, (*cur_command)[3]
//                };
//                three_args_instructions.at(cur_command->front())(args);
//            } else {
//                std::cerr << "Error: wrong command";
//            }
//        }
//
//        return {cache.get_hit_cnt(), cache.get_request_cnt()};
//    }
//private:
//    static void lui(InterpreterTwoArgsCommandArgs& args) {
//        uint32_t val = convert_to_unsigned_value(args.arg_2) << 11;
//        args.registers[args.arg_1] = val;
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void auipc(InterpreterTwoArgsCommandArgs& args) {
//        uint32_t address = args.registers["pc"] + (convert_to_unsigned_value(args.arg_2) << 11);
//        args.registers[args.arg_1] = address;
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void jal(InterpreterTwoArgsCommandArgs& args) {
//        int shift = expand_21_bit_value(args.arg_2) / 4;
//        args.registers[args.arg_1] = args.registers["pc"] + 4;
//        jump(args.registers, args.cur_command, shift);
//    }
//
//    static void mul(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = (int)args.registers[args.arg_2] * (int)args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void mulh(InterpreterThreeArgsCommandArgs& args) {
//        uint64_t res = (int64_t)args.registers[args.arg_2] * (int64_t)args.registers[args.arg_3];
//        args.registers[args.arg_1] = res >> 32; // todo maby int>>shift
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void mulhsu(InterpreterThreeArgsCommandArgs& args) {
//        uint64_t res = (uint64_t)args.registers[args.arg_2] * (uint64_t)args.registers[args.arg_3];
//        args.registers[args.arg_1] = res >> 32;
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void mulhu(InterpreterThreeArgsCommandArgs& args) {
//        uint64_t res = (int64_t)args.registers[args.arg_2] * (uint64_t)args.registers[args.arg_3];
//        args.registers[args.arg_1] = res >> 32;
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void div(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = (int)args.registers[args.arg_2] / (int)args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void divu(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] / args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void rem(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = (int)args.registers[args.arg_2] % (int)args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void remu(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] % args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void jalr(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t new_address = args.registers[args.arg_2] + expand_12_bit_value(args.arg_3);
//        new_address = new_address << 1 >> 1;
//        args.registers[args.arg_1] = args.registers["pc"] + 4;
//        int shift = ((int)new_address - (int)args.registers["pc"]) / 4;
//        jump(args.registers, args.cur_command, shift);
//    }
//
//    static void beq(InterpreterThreeArgsCommandArgs& args) {
//        if (args.registers[args.arg_1] == args.registers[args.arg_2]) {
//            return jump(args.registers, args.cur_command, expand_13_bit_value(args.arg_3) / 4);
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void bne(InterpreterThreeArgsCommandArgs& args) {
//        if (args.registers[args.arg_1] != args.registers[args.arg_2]) {
//            return jump(args.registers, args.cur_command, expand_13_bit_value(args.arg_3) / 4);
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void blt(InterpreterThreeArgsCommandArgs& args) {
//        if ((int)args.registers[args.arg_1] < (int)args.registers[args.arg_2]) {
//            return jump(args.registers, args.cur_command, expand_13_bit_value(args.arg_3) / 4);
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void bge(InterpreterThreeArgsCommandArgs& args) {
//        if ((int)args.registers[args.arg_1] >= (int)args.registers[args.arg_2]) {
//            return jump(args.registers, args.cur_command, expand_13_bit_value(args.arg_3) / 4);
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void bltu(InterpreterThreeArgsCommandArgs& args) {
//        if ((uint32_t)args.registers[args.arg_1] < (uint32_t)args.registers[args.arg_2]) {
//            return jump(args.registers, args.cur_command, expand_13_bit_value(args.arg_3) / 4);
//            args.cur_command += expand_12_bit_value(args.arg_3);
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void bgeu(InterpreterThreeArgsCommandArgs& args) {
//        if ((uint32_t)args.registers[args.arg_1] >= (uint32_t)args.registers[args.arg_2]) {
//            return jump(args.registers, args.cur_command, expand_13_bit_value(args.arg_3) / 4);
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void lb(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t address = args.registers[args.arg_3] + expand_12_bit_value(args.arg_2);
//        args.registers[args.arg_1] = expand_signed_byte(args.cache.read_1_byte(address));
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void lh(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t address = args.registers[args.arg_3] + expand_12_bit_value(args.arg_2);
//        args.registers[args.arg_1] = expand_2_signed_byte(args.cache.read_2_bytes(address));
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void lw(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t address = args.registers[args.arg_3] + expand_12_bit_value(args.arg_2);
//        args.registers[args.arg_1] = args.cache.read_4_bytes(address);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void lbu(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t address = args.registers[args.arg_3] + expand_12_bit_value(args.arg_2);
//        args.registers[args.arg_1] = args.cache.read_1_byte(address);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void lhu(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t address = args.registers[args.arg_3] + expand_12_bit_value(args.arg_2);
//        args.registers[args.arg_1] = args.cache.read_2_bytes(address);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void sb(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t address = args.registers[args.arg_3] + expand_12_bit_value(args.arg_2);
//        uint32_t byte_mask = (1 << 8) - 1;
//        uint8_t byte = args.registers[args.arg_1] & byte_mask;
//        args.cache.write_1_byte(address, byte);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void sh(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t address = args.registers[args.arg_3] + expand_12_bit_value(args.arg_2);
//        uint32_t half_word_mask = (1 << 16) - 1;
//        uint16_t half_word = args.registers[args.arg_1] & half_word_mask;
//        args.cache.write_2_bytes(address, half_word);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void sw(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t address = args.registers[args.arg_3] + expand_12_bit_value(args.arg_2);
//        uint32_t word = args.registers[args.arg_1];
//        args.cache.write_4_bytes(address, word);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void addi(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] + expand_12_bit_value(args.arg_3);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void slti(InterpreterThreeArgsCommandArgs& args) {
//        if ((int)args.registers[args.arg_2] < expand_12_bit_value(args.arg_3)) {
//            args.registers[args.arg_1] = 1;
//        } else {
//            args.registers[args.arg_1] = 0;
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    // todo check signed and unsigned <
//    static void sltiu(InterpreterThreeArgsCommandArgs& args) {
//        if (args.registers[args.arg_2] < convert_to_unsigned_value(args.arg_3)) {
//            args.registers[args.arg_1] = 1;
//        } else {
//            args.registers[args.arg_1] = 0;
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void xori(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] ^ expand_12_bit_value(args.arg_3);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void ori(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] | expand_12_bit_value(args.arg_3);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void andi(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] & expand_12_bit_value(args.arg_3);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void slli(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] << convert_to_unsigned_value(args.arg_3);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void srli(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] >> convert_to_unsigned_value(args.arg_3);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void srai(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = (int)args.registers[args.arg_2] >> convert_to_unsigned_value(args.arg_3);
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void add(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] + args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void sub(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] - args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void sll(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t shift_mask = (1 << 5) - 1;
//        uint32_t shift = args.registers[args.arg_3] & shift_mask;
//        args.registers[args.arg_1] = args.registers[args.arg_2] << shift;
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void slt(InterpreterThreeArgsCommandArgs& args) { // todo: add to map
//        if ((int)args.registers[args.arg_2] < (int)args.registers[args.arg_3]) {
//            args.registers[args.arg_1] = 1;
//        } else {
//            args.registers[args.arg_1] = 0;
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void sltu(InterpreterThreeArgsCommandArgs& args) { // todo: add to map
//        if (args.registers[args.arg_2] < args.registers[args.arg_3]) {
//            args.registers[args.arg_1] = 1;
//        } else {
//            args.registers[args.arg_1] = 0;
//        }
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void xor_(InterpreterThreeArgsCommandArgs& args) { // todo: add to map
//        args.registers[args.arg_1] = args.registers[args.arg_2] ^ args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void srl(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t shift_mask = (1 << 5) - 1;
//        uint32_t shift = args.registers[args.arg_3] & shift_mask;
//        args.registers[args.arg_1] = args.registers[args.arg_2] >> shift;
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void sra(InterpreterThreeArgsCommandArgs& args) {
//        uint32_t shift_mask = (1 << 5) - 1;
//        uint32_t shift = args.registers[args.arg_3] & shift_mask;
//        args.registers[args.arg_1] = (int)args.registers[args.arg_2] >> shift;
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void or_(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] | args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//
//    static void and_(InterpreterThreeArgsCommandArgs& args) {
//        args.registers[args.arg_1] = args.registers[args.arg_2] & args.registers[args.arg_3];
//        jump(args.registers, args.cur_command, 1);
//    }
//private:
//    typedef void (ThreeArgsInstruction)(InterpreterThreeArgsCommandArgs&);
//    static const std::unordered_map<std::string, ThreeArgsInstruction*> three_args_instructions;
//    typedef void (TwoArgsInstruction)(InterpreterTwoArgsCommandArgs&);
//    static const std::unordered_map<std::string, TwoArgsInstruction*> two_args_instructions;
//    using Commands = std::vector<std::vector<std::string>>;
//    Commands commands;
//    Commands::iterator cur_command;
//    Registers registers;
//    Cache& cache;
//private:
//    static int32_t expand_12_bit_value(const std::string& value) {
//        uint32_t ivalue = convert_to_unsigned_value(value);
//        if ((ivalue >> 11) & 1) {
//            uint32_t mask = 0xfffff000;
//            ivalue |= mask;
//        }
//        return static_cast<int>(ivalue);
//    }
//
//    static int32_t expand_13_bit_value(const std::string& value) {
//        uint32_t ivalue = convert_to_unsigned_value(value);
//        if ((ivalue >> 12) & 1) {
//            uint32_t mask = 0xfffff000;
//            ivalue |= mask;
//        }
//        return static_cast<int>(ivalue);
//    }
//
//    static int32_t expand_21_bit_value(const std::string& value) {
//        uint32_t ivalue = convert_to_unsigned_value(value);
//        if ((ivalue >> 20) & 1) {
//            uint32_t mask = 0xfff00000;
//            ivalue |= mask;
//        }
//        return static_cast<int>(ivalue);
//    }
//
//    static int32_t expand_signed_byte(uint8_t byte) {
//        uint32_t ibyte = byte;
//        if ((ibyte >> 7) & 1) {
//            uint32_t mask = 0xfffffff0;
//            ibyte |= mask;
//        }
//        return static_cast<int>(ibyte);
//    }
//
//    static int32_t expand_2_signed_byte(uint16_t byte) {
//        uint32_t ibyte = byte;
//        if ((ibyte >> 15) & 1) {
//            uint32_t mask = 0xffffff00;
//            ibyte |= mask;
//        }
//        return static_cast<int>(ibyte);
//    }
//
//    static uint32_t convert_to_unsigned_value(const std::string& value) {
//        uint32_t ivalue;
//        if (value.size() > 2 && value[1] == 'x') {
//            ivalue = std::stoul(value, nullptr, 16);
//        } else {
//            ivalue = std::stoul(value, nullptr, 10);
//        }
//        return ivalue;
//    }
//
//    static void jump(Registers& regs, Commands::iterator& cur, int shift) {
//        cur += shift;
//        regs["pc"] += 4 * shift;
//    }
//};
//
//const std::unordered_map<std::string, Interpreter::ThreeArgsInstruction*> Interpreter::three_args_instructions = {
//        {"jalr", &Interpreter::jalr}, {"beq", &Interpreter::beq}, {"bne", &Interpreter::bne},
//        {"blt", &Interpreter::blt}, {"bge", &Interpreter::bge}, {"bltu", &Interpreter::bltu},
//        {"bgeu", &Interpreter::bgeu}, {"lb", &Interpreter::lb}, {"lh", &Interpreter::lh}, {"lw", &Interpreter::lw},
//        {"lbu", &Interpreter::lbu}, {"lhu", &Interpreter::lhu}, {"sb", &Interpreter::sb}, {"sh", &Interpreter::sh},
//        {"sw", &Interpreter::sw}, {"addi", &Interpreter::addi}, {"slti", &Interpreter::slti},
//        {"sltiu", &Interpreter::sltiu}, {"xori", &Interpreter::xori}, {"ori", &Interpreter::ori},
//        {"andi", &Interpreter::andi}, {"slli", &Interpreter::slli}, {"srli", &Interpreter::srli},
//        {"srai", &Interpreter::srai}, {"add", &Interpreter::add}, {"sub", &Interpreter::sub},
//        {"sll", &Interpreter::sll},{"slt", &Interpreter::slt}, {"sltu", &Interpreter::sltu},{"xor", &Interpreter::xor_},
//        {"srl", &Interpreter::srl}, {"sra", &Interpreter::sra}, {"or", &Interpreter::or_}, {"and", &Interpreter::and_},
//        {"mul", &Interpreter::mul}, {"mulh", &Interpreter::mulh}, {"mulhsu", &Interpreter::mulhsu},
//        {"mulhu", &Interpreter::mulhu}, {"div", &Interpreter::div}, {"divu", &Interpreter::divu},
//        {"rem", &Interpreter::rem}, {"remu", &Interpreter::remu}
//};
//
//const std::unordered_map<std::string, Interpreter::TwoArgsInstruction*> Interpreter::two_args_instructions = {
//        {"lui", &Interpreter::lui}, {"auipc", &Interpreter::auipc}, {"auipc", &Interpreter::auipc},
//        {"jal", &Interpreter::jal}
//};
//
//
//class InstructionEncoder {
//public:
//    InstructionEncoder& add_bits(uint8_t bits_cnt, uint32_t bits) {
//        while(bits_cnt--) {
//            bin_code.set(--cur_index, (bits >> bits_cnt) & 1);
//        }
//        return *this;
//    }
//
//    InstructionEncoder& add_register(const std::string& reg_name) {
//        uint8_t reg_code = from_reg_name_to_code.at(reg_name);
//        add_bits(5, reg_code);
//        return *this;
//    }
//
//    uint32_t get_code() {
//        return bin_code.to_ulong();
//    }
//private:
//    static const std::unordered_map<std::string, uint8_t> from_reg_name_to_code;
//    std::bitset<32> bin_code;
//    uint8_t cur_index = 32;
//};
//
//const std::unordered_map<std::string, uint8_t> InstructionEncoder::from_reg_name_to_code = {
//        {"x0", 0}, {"x1", 1}, {"x2", 2}, {"x3", 3}, {"x4", 4}, {"x5", 5}, {"x6", 6}, {"x7", 7}, {"x8", 8}, {"x9", 9},
//        {"x10", 10}, {"x11", 11}, {"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15}, {"x16", 16}, {"x17", 17},
//        {"x18", 18}, {"x19", 19}, {"x20", 20}, {"x21", 21}, {"x22", 22}, {"x23", 23}, {"x24", 24}, {"x25", 25},
//        {"x26", 26}, {"x27", 27}, {"x28", 28}, {"x29", 29}, {"x30", 30}, {"x31", 31}, {"zero", 0}, {"ra", 1},
//        {"sp", 2}, {"gp", 3}, {"tp", 4}, {"t0", 5}, {"t1", 6}, {"t2", 7}, {"s0", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11},
//        {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19}, {"s4", 20},
//        {"s5", 21}, {"s6", 22}, {"s7", 23}, {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
//        {"t5", 30}, {"t6", 31}, {"fp", 8}
//};
//
//struct EncoderTwoArgsCommandArgs {
//    std::string& arg_1;
//    std::string& arg_2;
//};
//
//struct EncoderThreeArgsCommandArgs : public EncoderTwoArgsCommandArgs{
//    std::string& arg_3;
//};
//
//
//class CodeEncoder {
//public:
//    explicit CodeEncoder(std::vector<std::vector<std::string>> commands)
//            : commands(std::move(commands)) {
//
//    }
//
//    void encode_all_commands(const std::filesystem::path& out_file_path) {
//        std::ofstream fout(out_file_path, std::ios::binary);
//        for (auto& command: commands) {
//            uint32_t code;
//            if (two_args_instructions.contains(command[0])) {
//                EncoderTwoArgsCommandArgs args{command[1], command[2]};
//                code = two_args_instructions.at(command[0])(args);
//            } else if (three_args_instructions.contains(command[0])) {
//                EncoderThreeArgsCommandArgs args{{command[1], command[2]}, command[3]};
//                code = three_args_instructions.at(command[0])(args);
//            } else {
//                std::cerr << "Error: command " << command[0] << " not found" << std::endl;
//                exit(-1);
//            }
//            uint32_t byte_mask = (1 << 8) - 1;
//            uint8_t byte_0 = (code >> 24) & byte_mask;
//            uint8_t byte_1 = (code >> 16) & byte_mask;
//            uint8_t byte_2 = (code >> 8) & byte_mask;
//            uint8_t byte_3 = code & byte_mask;
//            fout.put(byte_3);
//            fout.put(byte_2);
//            fout.put(byte_1);
//            fout.put(byte_0);
//        }
//    }
//private:
//    static uint32_t lui(EncoderTwoArgsCommandArgs& args) {
//        uint32_t imm = convert_to_unsigned_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(20, imm).add_register(args.arg_1)
//                .add_bits(7, 0b0110111).get_code();
//        return code;
//    }
//
//    static uint32_t auipc(EncoderTwoArgsCommandArgs& args) {
//        uint32_t imm = convert_to_unsigned_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(20, imm).add_register(args.arg_1)
//                .add_bits(7, 0b0010111).get_code();
//        return code;
//    }
//
//    static uint32_t jal(EncoderTwoArgsCommandArgs& args) {
//        uint32_t imm = expand_21_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(1, slice(imm, 20, 20)).add_bits(10, slice(imm, 10, 1))
//                .add_bits(1, slice(imm, 11, 11)).add_bits(8, slice(imm, 19, 12))
//                .add_register(args.arg_1).add_bits(7, 0b1101111).get_code();
//        return code;
//    }
//
//    static uint32_t mul(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000001).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b000).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t mulh(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000001).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b001).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t mulhsu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000001).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b010).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t mulhu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000001).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b011).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t div(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000001).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b100).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t divu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000001).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b101).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t rem(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000001).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b110).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t remu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000001).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b111).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t jalr(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b000)
//                .add_register(args.arg_1).add_bits(7, 0b1100111).get_code();
//        return code;
//    }
//
//    static uint32_t beq(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_13_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(1, slice(imm, 12, 12)).add_bits(6, slice(imm, 10, 5))
//                .add_register(args.arg_2).add_register(args.arg_1).add_bits(3, 0b000).add_bits(4, slice(imm, 4, 1))
//                .add_bits(1, slice(imm, 11, 11)).add_bits(7, 0b1100011).get_code();
//        return code;
//    }
//
//    static uint32_t bne(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_13_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(1, slice(imm, 12, 12)).add_bits(6, slice(imm, 10, 5))
//                .add_register(args.arg_2).add_register(args.arg_1).add_bits(3, 0b001).add_bits(4, slice(imm, 4, 1))
//                .add_bits(1, slice(imm, 11, 11)).add_bits(7, 0b1100011).get_code();
//        return code;
//
//    }
//
//    static uint32_t blt(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_13_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(1, slice(imm, 12, 12)).add_bits(6, slice(imm, 10, 5))
//                .add_register(args.arg_2).add_register(args.arg_1).add_bits(3, 0b100).add_bits(4, slice(imm, 4, 1))
//                .add_bits(1, slice(imm, 11, 11)).add_bits(7, 0b1100011).get_code();
//        return code;
//    }
//
//    static uint32_t bge(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_13_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(1, slice(imm, 12, 12)).add_bits(6, slice(imm, 10, 5))
//                .add_register(args.arg_2).add_register(args.arg_1).add_bits(3, 0b101).add_bits(4, slice(imm, 4, 1))
//                .add_bits(1, slice(imm, 11, 11)).add_bits(7, 0b1100011).get_code();
//        return code;
//    }
//
//    static uint32_t bltu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_13_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(1, slice(imm, 12, 12)).add_bits(6, slice(imm, 10, 5))
//                .add_register(args.arg_2).add_register(args.arg_1).add_bits(3, 0b110).add_bits(4, slice(imm, 4, 1))
//                .add_bits(1, slice(imm, 11, 11)).add_bits(7, 0b1100011).get_code();
//        return code;
//    }
//
//    static uint32_t bgeu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_13_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(1, slice(imm, 12, 12)).add_bits(6, slice(imm, 10, 5))
//                .add_register(args.arg_2).add_register(args.arg_1).add_bits(3, 0b111).add_bits(4, slice(imm, 4, 1))
//                .add_bits(1, slice(imm, 11, 11)).add_bits(7, 0b1100011).get_code();
//        return code;
//    }
//
//    static uint32_t lb(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_3).add_bits(3, 0b000)
//                .add_register(args.arg_1).add_bits(7, 0b0000011).get_code();
//        return code;
//    }
//
//    static uint32_t lh(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_3).add_bits(3, 0b001)
//                .add_register(args.arg_1).add_bits(7, 0b0000011).get_code();
//        return code;
//    }
//
//    static uint32_t lw(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_3).add_bits(3, 0b010)
//                .add_register(args.arg_1).add_bits(7, 0b0000011).get_code();
//        return code;
//    }
//
//    static uint32_t lbu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_3).add_bits(3, 0b100)
//                .add_register(args.arg_1).add_bits(7, 0b0000011).get_code();
//        return code;    }
//
//    static uint32_t lhu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_3).add_bits(3, 0b101)
//                .add_register(args.arg_1).add_bits(7, 0b0000011).get_code();
//        return code;
//    }
//
//    static uint32_t sb(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(7, slice(imm, 11, 5)).add_register(args.arg_1)
//                .add_register(args.arg_3).add_bits(3, 0b000).add_bits(5, slice(imm, 4, 0))
//                .add_bits(7, 0b0100011).get_code();
//        return code;
//    }
//
//    static uint32_t sh(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(7, slice(imm, 11, 5)).add_register(args.arg_1)
//                .add_register(args.arg_3).add_bits(3, 0b001).add_bits(5, slice(imm, 4, 0))
//                .add_bits(7, 0b0100011).get_code();
//        return code;
//    }
//
//    static uint32_t sw(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_2);
//        uint32_t code = InstructionEncoder().add_bits(7, slice(imm, 11, 5)).add_register(args.arg_1)
//                .add_register(args.arg_3).add_bits(3, 0b010).add_bits(5, slice(imm, 4, 0))
//                .add_bits(7, 0b0100011).get_code();
//        return code;
//    }
//
//    static uint32_t addi(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b000)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    static uint32_t slti(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b010)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    // todo check signed and unsigned <
//    static uint32_t sltiu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b011)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    static uint32_t xori(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b100)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    static uint32_t ori(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b110)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    static uint32_t andi(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = expand_12_bit_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b111)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    static uint32_t slli(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = convert_to_unsigned_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b001)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    static uint32_t srli(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = convert_to_unsigned_value(args.arg_3);
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b101)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    static uint32_t srai(EncoderThreeArgsCommandArgs& args) {
//        uint32_t imm = convert_to_unsigned_value(args.arg_3);
//        imm |= 1 << 10;
//        uint32_t code = InstructionEncoder().add_bits(12, imm).add_register(args.arg_2).add_bits(3, 0b101)
//                .add_register(args.arg_1).add_bits(7, 0b0010011).get_code();
//        return code;
//    }
//
//    static uint32_t add(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b000).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t sub(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0100000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b000).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t sll(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b001).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t slt(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b010).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t sltu(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b011).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t xor_(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b100).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t srl(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b101).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t sra(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0100000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b101).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t or_(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b110).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//
//    static uint32_t and_(EncoderThreeArgsCommandArgs& args) {
//        uint32_t code = InstructionEncoder().add_bits(7, 0b0000000).add_register(args.arg_3).add_register(args.arg_2)
//                .add_bits(3, 0b111).add_register(args.arg_1).add_bits(7, 0b0110011).get_code();
//        return code;
//    }
//private:
//    static int32_t expand_12_bit_value(const std::string& value) {
//        uint32_t ivalue = convert_to_unsigned_value(value);
//        if ((ivalue >> 11) & 1) {
//            uint32_t mask = 0xfffff000;
//            ivalue |= mask;
//        }
//        return static_cast<int>(ivalue);
//    }
//
//    static int32_t expand_13_bit_value(const std::string& value) {
//        uint32_t ivalue = convert_to_unsigned_value(value);
//        if ((ivalue >> 12) & 1) {
//            uint32_t mask = 0xfffff000;
//            ivalue |= mask;
//        }
//        return static_cast<int>(ivalue);
//    }
//
//    static int32_t expand_21_bit_value(const std::string& value) {
//        uint32_t ivalue = convert_to_unsigned_value(value);
//        if ((ivalue >> 20) & 1) {
//            uint32_t mask = 0xfff00000;
//            ivalue |= mask;
//        }
//        return static_cast<int>(ivalue);
//    }
//
//
//    static uint32_t convert_to_unsigned_value(const std::string& value) {
//        uint32_t ivalue;
//        if (value.size() > 2 && value[1] == 'x') {
//            ivalue = std::stoul(value, nullptr, 16);
//        } else {
//            ivalue = std::stoul(value, nullptr, 10);
//        }
//        return ivalue;
//    }
//
//    static uint32_t slice(uint32_t value, uint8_t first, uint8_t last) {
//        value >>= last;
//        value &= (1 << (first - last + 1)) - 1;
//        return value;
//    }
//public:
//    typedef uint32_t (*ThreeArgsInstruction)(EncoderThreeArgsCommandArgs&);
//    static const std::unordered_map<std::string, ThreeArgsInstruction> three_args_instructions;
//    typedef uint32_t (*TwoArgsInstruction)(EncoderTwoArgsCommandArgs&);
//    static const std::unordered_map<std::string, TwoArgsInstruction> two_args_instructions;
//    using Commands = std::vector<std::vector<std::string>>;
//    Commands commands;
//};
//
//
//const std::unordered_map<std::string, CodeEncoder::ThreeArgsInstruction> CodeEncoder::three_args_instructions = {
//        {"jalr", &CodeEncoder::jalr}, {"beq", &CodeEncoder::beq}, {"bne", &CodeEncoder::bne},
//        {"blt", &CodeEncoder::blt}, {"bge", &CodeEncoder::bge}, {"bltu", &CodeEncoder::bltu},
//        {"bgeu", &CodeEncoder::bgeu}, {"lb", &CodeEncoder::lb}, {"lh", &CodeEncoder::lh}, {"lw", &CodeEncoder::lw},
//        {"lbu", &CodeEncoder::lbu}, {"lhu", &CodeEncoder::lhu}, {"sb", &CodeEncoder::sb}, {"sh", &CodeEncoder::sh},
//        {"sw", &CodeEncoder::sw}, {"addi", &CodeEncoder::addi}, {"slti", &CodeEncoder::slti},
//        {"sltiu", &CodeEncoder::sltiu}, {"xori", &CodeEncoder::xori}, {"ori", &CodeEncoder::ori},
//        {"andi", &CodeEncoder::andi}, {"slli", &CodeEncoder::slli}, {"srli", &CodeEncoder::srli},
//        {"srai", &CodeEncoder::srai}, {"add", &CodeEncoder::add}, {"sub", &CodeEncoder::sub},
//        {"sll", &CodeEncoder::sll}, {"srl", &CodeEncoder::srl}, {"sra", &CodeEncoder::sra},
//        {"or", &CodeEncoder::or_}, {"and", &CodeEncoder::and_}, {"mul", &CodeEncoder::mul},
//        {"mulh", &CodeEncoder::mulh}, {"mulhsu", &CodeEncoder::mulhsu}, {"mulhu", &CodeEncoder::mulhu},
//        {"div", &CodeEncoder::div}, {"divu", &CodeEncoder::divu}, {"rem", &CodeEncoder::rem},
//        {"remu", &CodeEncoder::remu}, {"slt", &CodeEncoder::slt}, {"sltu", &CodeEncoder::sltu},
//        {"xor", &CodeEncoder::xor_}
//};
//#include <ranges>
//const std::unordered_map<std::string, CodeEncoder::TwoArgsInstruction> CodeEncoder::two_args_instructions = {
//        {"lui", &CodeEncoder::lui}, {"auipc", &CodeEncoder::auipc}, {"auipc", &CodeEncoder::auipc},
//        {"jal", &CodeEncoder::jal}
//};
//
//class CommandLineArgsParser {
//public:
//    CommandLineArgsParser(int argc, char** argv)
//            : argc(argc), argv(argv), cur_pos(1) {
//        args = {{"replacement", ""}, {"asm", ""}, {"bin", ""}};
//    }
//
//    void parse_all_arguments() {
//        while (cur_pos != argc) {
//            ParseOneArgument();
//        }
//    }
//
//    std::tuple<int, const std::string&, const std::string&> get_arguments() {
//        int replacement = args["replacement"][0] - '0';
//        return {replacement, args["asm"], args["bin"]};
//    }
//private:
//    int argc;
//    char** argv;
//    int cur_pos;
//    std::unordered_map<std::string, std::string> args;
//
//    void ParseOneArgument() {
//        if (argv[cur_pos][0] != '-' || argv[cur_pos][1] != '-') {
//            std::cerr << "Wrong command line argument format: " << argv[cur_pos] << std::endl;
//        }
//        std::string debug(argv[cur_pos + 1]);
//        args.at(std::string(argv[cur_pos] + 2)) = std::string(argv[cur_pos + 1]);
//        cur_pos += 2;
//    }
//};

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
    std::ifstream asm_file(cmd_args_parser.getAsmFile());
    if (!asm_file.is_open()) {
        std::cerr << "failed to open " << cmd_args_parser.getAsmFile() << std::endl;
        exit(-1);
    }
    std::vector<std::vector<std::string>> commands;
    AsmCommandParser asm_command_parser(
            std::istreambuf_iterator<char>(asm_file)
            , std::istreambuf_iterator<char>()
            , std::back_inserter(commands)
    );
    int replacement = cmd_args_parser.getReplacement();
    if (replacement == 0 || replacement == 1) {
        LRUCache<CACHE_WAY, CACHE_TAG_LEN, CACHE_INDEX_LEN, CACHE_OFFSET_LEN> cache;
        Interpreter interpreter(commands, cache);
        auto [hits_cnt, requests_cnt] = interpreter.run();
//        double hits_percent = (double)hits_cnt / requests_cnt * 100;
//        std::printf("LRU\thit rate: %3.4f%%\n", hits_percent);
    }


//    if (replacement == 0 || replacement == 2) {
//        PLRUCache cache(CACHE_WAY, CACHE_TAG_LEN, CACHE_INDEX_LEN, CACHE_OFFSET_LEN);
//        Interpreter interpreter(commands, cache);
//        auto [hits_cnt, requests_cnt] = interpreter.run();
//        double hits_percent = (double)hits_cnt / requests_cnt * 100;
//        std::printf("pLRU\thit rate: %3.4f%%\n", hits_percent);
//    }
//    CodeEncoder code_encoder(commands);
//    code_encoder.encode_all_commands(bin_file_path);
    return 0;
}
