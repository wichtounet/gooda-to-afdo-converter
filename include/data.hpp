#ifndef CONVERTER_DATA_HPP
#define CONVERTER_DATA_HPP

#include <vector>
#include <string>

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
};

}

#endif
