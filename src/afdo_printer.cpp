#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <cstring>

#include "afdo_printer.hpp"

void gooda::dump_afdo(const afdo_data& data){
    std::cout << "The AFDO data contains " << data.functions.size() << " hotspot functions" << std::endl;

    for(auto& function : data.functions) {
        std::cout << function.name << " (" << function.file << ")" << " [" << function.total_count << ":" << function.entry_count << "]" << std::endl;

        for(auto& stack : function.stacks){
            std::cout << "   Stack of " << stack.num_inst << " instructions [" << stack.count << "]" << std::endl;

            for(auto& pos : stack.stack){
                std::cout << "      Instruction at line " << pos.line << " (" << pos.file << "), discr=" << pos.discr << std::endl;
            }
        }
    }

    std::cout << "Length" << std::endl;
    std::cout << "   File Name Table: " << (data.length_file_section * 4) << "B" << std::endl;
    std::cout << "   Function Table: " << (data.length_function_section * 4) << "B" << std::endl;
    std::cout << "   Modules Table: " << (data.length_modules_section * 4) << "B" << std::endl;
    std::cout << "   Working Set Table: " << (data.length_working_set_section * 4) << "B" << std::endl;
}
