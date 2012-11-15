#ifndef GOODA_DATA_HPP
#define GOODA_DATA_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <array>

#include "gcov_types.hpp"

namespace gooda {

static const unsigned int WS_SIZE = 128;

struct afdo_pos {
    std::string func = "";
    std::string file = "";
    gcov_unsigned_t line = 0;
    gcov_unsigned_t discr = 0;
};

struct afdo_stack {
    std::vector<afdo_pos> stack;
    gcov_type count = 0;
    gcov_type num_inst = 0;
};

struct afdo_function {
    std::string name = "";
    std::string file = "";
    gcov_type total_count = 0;
    gcov_type entry_count = 0;
    std::vector<afdo_stack> stacks;
};

struct afdo_module {
    std::string name = "";
    gcov_unsigned_t exported = 0;
    gcov_unsigned_t has_asm = 0;
    gcov_unsigned_t num_aux_modules = 0;
    gcov_unsigned_t num_quote_paths = 0;
    gcov_unsigned_t num_bracket_paths = 0;
    gcov_unsigned_t num_cpp_defines = 0;
    gcov_unsigned_t num_cpp_includes = 0;
    gcov_unsigned_t num_cl_args = 0;
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
    std::vector<afdo_working_set> working_set;

    unsigned int length_file_section = 0;
    unsigned int length_function_section = 0;
    unsigned int length_modules_section = 0;
    unsigned int length_working_set_section = 0;

    afdo_data();

    gcov_unsigned_t get_file_index(const std::string& file) const;
    void add_file_name(const std::string& file);
    
    private:
        std::unordered_map<std::string, gcov_unsigned_t> file_index;
};

}

#endif
