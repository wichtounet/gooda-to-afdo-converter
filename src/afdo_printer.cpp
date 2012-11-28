#include <iostream>
#include <sstream>

#include "afdo_printer.hpp"

std::string pretty_size(unsigned int size){
    std::stringstream stream;
    std::string unit = "B";

    if(size > 1024){
       size /= 1024; 
       unit = "KB";
    }
    
    if(size > 1024){
       size /= 1024; 
       unit = "MB";
    }

    stream << size << unit;
    return stream.str();
}

void gooda::dump_afdo(const afdo_data& data){
    std::cout << "The AFDO data contains " << data.functions.size() << " hotspot functions" << std::endl;

    for(std::size_t i = 0; i < data.file_names.size(); ++i){
        std::cout << i << ":" << data.file_names[i] << std::endl;
    }

    std::cout << "Hotspot functions" << std::endl;
    for(auto& function : data.functions) {
        std::cout << function.name << " (" << function.file << "(" << data.get_file_index(function.file) << "))" 
            << " [" << function.total_count << ":" << function.entry_count << "]" << std::endl;

        for(auto& stack : function.stacks){
            std::cout << "   Stack of " << stack.num_inst << " instructions [count=" << stack.count << ", misses=" << stack.cache_misses << "]" << std::endl;

            for(auto& pos : stack.stack){
                std::cout << "      Instruction at line " << pos.line 
                    << " (file=" << pos.file << "(" << data.get_file_index(pos.file) << "), func=" << pos.func << "(" << data.get_file_index(pos.func) << "))" 
                    << ", discr=" << pos.discr << std::endl;
            }
        }
    }

    std::cout << "Working Set" << std::endl;
    for(std::size_t i = 0; i < data.working_set.size(); ++i){
        std::cout << "   " << i << ": min= " << data.working_set[i].min_counter << " num= " << data.working_set[i].num_counter << std::endl;
    }

    std::cout << "Length" << std::endl;
    std::cout << "   File Name Table: " << pretty_size(data.length_file_section * 4) << std::endl;
    std::cout << "   Function Table: " << pretty_size(data.length_function_section * 4) << std::endl;
    std::cout << "   Modules Table: " << pretty_size(data.length_modules_section * 4) << std::endl;
    std::cout << "   Working Set Table: " << pretty_size(data.length_working_set_section * 4) << std::endl;
}

void gooda::dump_afdo_light(const afdo_data& data){
    std::cout << "The AFDO data contains " << data.functions.size() << " hotspot functions" << std::endl;

    for(auto& function : data.functions) {
        std::cout << function.name << " (" << function.file << ")" << " [" << function.total_count << ":" << function.entry_count << "]" << std::endl;
    }
}
