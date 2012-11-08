#ifndef CONVERTER_DATA_HPP
#define CONVERTER_DATA_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <array>

#include "gcov_types.hpp"

namespace converter {

static const unsigned int WS_SIZE = 128;

struct afdo_pos {
    std::string func;
    std::string file;
    gcov_unsigned_t line;
    gcov_unsigned_t discr;
};

struct afdo_stack {
    std::vector<afdo_pos> stack;
    gcov_type count;
    gcov_type num_inst;
};

struct afdo_function {
    std::string name;
    std::string file;
    gcov_type total_count;
    gcov_type entry_count;
    std::vector<afdo_stack> stacks;
};

struct afdo_module {
    std::string name;
    gcov_unsigned_t exported;
    gcov_unsigned_t has_asm;
    gcov_unsigned_t num_aux_modules;
    gcov_unsigned_t num_quote_paths;
    gcov_unsigned_t num_bracket_paths;
    gcov_unsigned_t num_cpp_defines;
    gcov_unsigned_t num_cpp_includes;
    gcov_unsigned_t num_cl_args;
    std::vector<std::string> strings;
};

struct afdo_working_set {
    gcov_unsigned_t num_counter;
    gcov_type min_counter;
};

struct afdo_data {
    std::vector<std::string> file_names;
    std::vector<afdo_function> functions;
    std::vector<afdo_module> modules;
    std::array<afdo_working_set, WS_SIZE> working_set;

    gcov_unsigned_t get_file_index(const std::string& file) const;
    void add_file_name(const std::string& file);
    
    private:
        std::unordered_map<std::string, gcov_unsigned_t> file_index;
};

}

#endif
