//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "afdo_reader.hpp"
#include "gcov_file.hpp"
#include "logger.hpp"

namespace {

void read_string_table(gooda::gcov_file& gcov_file, gooda::afdo_data& data){
    log::emit<log::Debug>() << "Read the string table" << log::endl;

    //AFDO TAG
    gcov_file.read_unsigned();

    //The length of the section
    data.length_file_section = gcov_file.read_unsigned();

    auto files = gcov_file.read_unsigned();

    for(gcov_unsigned_t i = 0; i < files; ++i){
        auto file_name = gcov_file.read_string();
        data.add_file_name(file_name);
    }
}

void read_function_profile(gooda::gcov_file& gcov_file, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    //AFDO TAG
    gcov_file.read_unsigned();

    //The length of the section
    data.length_function_section = gcov_file.read_unsigned();

    auto functions = gcov_file.read_unsigned();

    for(gcov_unsigned_t i = 0; i < functions; ++i){
        gooda::afdo_function function;
        function.name = gcov_file.read_string();
        function.file = data.file_name(gcov_file.read_unsigned());
        function.total_count = gcov_file.read_counter();
        function.entry_count = gcov_file.read_counter();

        auto stacks = gcov_file.read_unsigned();
        for(gcov_unsigned_t j = 0; j < stacks; ++j){
            gooda::afdo_stack stack;

            auto size = gcov_file.read_unsigned();
            for(gcov_unsigned_t k = 0; k < size; ++k){
                auto func = data.file_name(gcov_file.read_unsigned());
                auto file = data.file_name(gcov_file.read_unsigned());
                auto line = gcov_file.read_unsigned();
                auto disc = gcov_file.read_unsigned();

                stack.stack.emplace_back(std::move(func), std::move(file), line, disc);
            }

            stack.count = gcov_file.read_counter();
            stack.num_inst = gcov_file.read_counter();

            if(vm.count("cache-misses")){
                stack.cache_misses = gcov_file.read_counter();
            }

            function.stacks.push_back(std::move(stack));
        }

        data.functions.push_back(std::move(function));
    }
}

void read_module_info(gooda::gcov_file& gcov_file, gooda::afdo_data& data){
    //AFDO TAG
    gcov_file.read_unsigned();

    //The length of the section
    data.length_modules_section = gcov_file.read_unsigned();

    auto modules = gcov_file.read_unsigned();

    for(gcov_unsigned_t i = 0; i < modules; ++i){
        gooda::afdo_module module;

        module.name = gcov_file.read_string();
        module.exported = gcov_file.read_unsigned();
        module.has_asm = gcov_file.read_unsigned();
        module.num_aux_modules = gcov_file.read_unsigned();
        module.num_quote_paths = gcov_file.read_unsigned();
        module.num_bracket_paths = gcov_file.read_unsigned();
        module.num_cpp_defines = gcov_file.read_unsigned();
        module.num_cpp_includes = gcov_file.read_unsigned();
        module.num_cl_args = gcov_file.read_unsigned();

        auto strings = 
                module.num_aux_modules + module.num_quote_paths + module.num_bracket_paths 
            +   module.num_cpp_defines + module.num_cpp_includes + module.num_cl_args;

        for(gcov_unsigned_t j = 0; j < strings; ++j){
            module.strings.push_back(gcov_file.read_string());
        }

        data.modules.push_back(std::move(module));
    }
}

void read_working_set(gooda::gcov_file& gcov_file, gooda::afdo_data& data){
    //AFDO TAG
    gcov_file.read_unsigned();

    //The length of the section
    data.length_working_set_section = gcov_file.read_unsigned();

    for(unsigned int i = 0; i < gooda::WS_SIZE; ++i){
        data.working_set[i].num_counter = gcov_file.read_unsigned();
        data.working_set[i].min_counter = gcov_file.read_counter();
    }
}

} //end of anonymous

void gooda::read_afdo(const std::string& afdo_file, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    gooda::gcov_file gcov_file;

    if(!gcov_file.open_for_read(afdo_file)){
        std::cout << "Cannot open file for reading" << std::endl;
        return;
    }

    //Magic number
    gcov_file.read_unsigned();
    
    //GCOV Version
    gcov_file.read_unsigned();
    
    //Unused unsigned
    gcov_file.read_unsigned();

    //Read the different sections
    read_string_table(gcov_file, data);
    read_function_profile(gcov_file, data, vm);
    read_module_info(gcov_file, data);
    read_working_set(gcov_file, data);
}
