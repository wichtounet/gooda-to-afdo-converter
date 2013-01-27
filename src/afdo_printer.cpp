//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file afdo_printer.cpp
 * \brief Implementation of the dumping of AFDO the console. 
 */

#include <iostream>
#include <sstream>

#include "afdo_printer.hpp"

namespace {

/*!
 * \brief Generate a pretty string from the size, including unit
 * \param size The size in bytes
 * \return A string representation of the size.
 */
std::string pretty_size(unsigned int size){
    std::stringstream stream;
    std::string unit = "B";

    stream << size << unit;

    if(size > 1024){
       size /= 1024; 
       unit = "KB";
    }
    
    if(size > 1024){
       size /= 1024; 
       unit = "MB";
    }

    if(unit != "B"){
        stream << "(" << size << unit << ")";
    }

    return stream.str();
}

/*!
 * \brief Print a file with a verbosity depending on the configuration. 
 * \param data The AFDO profile. 
 * \param vm The configuration. 
 * \param file The file to print. 
 */
void print_file(const gooda::afdo_data& data, boost::program_options::variables_map& vm, const std::string& file){
    if(vm.count("debug")){
        std::cout << file << "(" << data.get_file_index(file) << ")"; 
    } else {
        std::cout << file; 
    }
}

} //end of anonymous namespace

void gooda::dump_afdo(const afdo_data& data, boost::program_options::variables_map& vm){
    std::cout << "The AFDO data contains " << data.functions.size() << " hotspot functions" << std::endl;

    std::cout << "Strings" << std::endl;
    for(std::size_t i = 0; i < data.file_names.size(); ++i){
        std::cout << "   " << i << ":" << data.file_names[i] << std::endl;
    }

    auto functions = data.functions;

    std::sort(functions.begin(), functions.end(), [](const gooda::afdo_function& lhs, const gooda::afdo_function& rhs){return lhs.total_count > rhs.total_count; });

    std::cout << "Hotspot functions" << std::endl;
    for(auto& function : functions) {
        std::cout << function.name << " (";
        print_file(data, vm, function.file);
        std::cout << ")" << " [" << function.total_count << ":" << function.entry_count << "]" << std::endl;

        auto stacks = function.stacks;

        std::sort(stacks.begin(), stacks.end(), [](const gooda::afdo_stack& lhs, const gooda::afdo_stack& rhs){
                    return lhs.stack.empty() || rhs.stack.empty() ? false : lhs.stack.back().line < rhs.stack.back().line; 
                });

        for(auto& stack : stacks){
            if(stack.stack.empty()){
                std::cout << "   INVALID STACK of size " << stack.stack.size() 
                    << ", with " << stack.num_inst << " dynamic instructions " 
                    << "[count=" << stack.count << "]" << std::endl;
            } else {
                std::cout << "   Stack of size " << stack.stack.size() 
                    << ", with " << stack.num_inst << " dynamic instructions " 
                    << "[count=" << stack.count;

                if(vm.count("cache-misses")){
                    std::cout << ", cache-misses=" << stack.cache_misses;
                }

                std::cout << "]" << std::endl;

                for(auto& pos : stack.stack){
                    std::cout << "      Instruction at ";
                    print_file(data, vm, pos.file);
                    std::cout << ":" << pos.line << ", func=";
                    print_file(data, vm, pos.func);
                    std::cout << ", discr=" << pos.discriminator << std::endl;
                }
            }
        }

        std::cout << std::endl;
    }

    if(!vm.count("nows")){
        std::cout << "Working Set" << std::endl;
        for(std::size_t i = 0; i < data.working_set.size(); ++i){
            std::cout << "   " << i << ": min= " << data.working_set[i].min_counter << " num= " << data.working_set[i].num_counter << std::endl;
        }
    }

    std::cout << "Length" << std::endl;
    std::cout << "   File Name Table: " << pretty_size(data.length_file_section * 4) << std::endl;
    std::cout << "   Function Table: " << pretty_size(data.length_function_section * 4) << std::endl;
    std::cout << "   Modules Table: " << pretty_size(data.length_modules_section * 4) << std::endl;
    std::cout << "   Working Set Table: " << pretty_size(data.length_working_set_section * 4) << std::endl;
}

void gooda::dump_afdo_light(const afdo_data& data, boost::program_options::variables_map& /*vm*/){
    std::cout << "The AFDO data contains " << data.functions.size() << " hotspot functions" << std::endl;

    auto functions = data.functions;
   
    std::sort(functions.begin(), functions.end(), [](const gooda::afdo_function& lhs, const gooda::afdo_function& rhs){return lhs.total_count > rhs.total_count; });

    for(auto& function : functions) {
        std::cout << "   " << function.name << " (" << function.file << ")" << " [" << function.total_count << ":" << function.entry_count << "]" << std::endl;
    }
}
