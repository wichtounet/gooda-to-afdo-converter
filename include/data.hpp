#ifndef CONVERTER_DATA_HPP
#define CONVERTER_DATA_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "gcov_types.hpp"

namespace converter {

struct CallSitePos {
    std::string func;
    std::string file;
    gcov_unsigned_t line;
    gcov_unsigned_t discr;
};

struct Stack {
    std::vector<CallSitePos> stack;
    gcov_type count;
    gcov_type num_inst;
};

struct Function {
    std::string name;
    std::string file;
    gcov_type total_count;
    gcov_type entry_count;
    std::vector<Stack> stacks;
};

struct Data {
    std::vector<std::string> file_names;
    std::vector<Function> functions;

    gcov_unsigned_t get_file_index(const std::string& file);
    void add_file_name(const std::string& file);
    
    private:
        std::unordered_map<std::string, gcov_unsigned_t> file_index;
};

}

#endif
