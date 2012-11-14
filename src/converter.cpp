#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include "converter.hpp"
#include "utils.hpp"

namespace {

void read_asm_file(const gooda::gooda_report& report, std::size_t i, gooda::afdo_data& data){
    if(report.has_asm_file(i)){
        auto& function = data.functions[i];
        auto& file = report.asm_file(i);

        for(auto& line : file){
            auto disassembly = line.get_string(ASM_DISASSEMBLY);
            
            //Get the entry basic block
            if(boost::starts_with(disassembly, "Basic Block 1 <")){
                function.entry_count = line.get_counter(ASM_UNHALTED_CORE_CYCLES);

                break;
            }
        }
    }
}

void read_src_file(const gooda::gooda_report& report, std::size_t i, gooda::afdo_data& data){
    if(report.has_src_file(i)){
        auto& function = data.functions[i];

        auto& file = report.src_file(i);

        for(auto& line : file){
            auto line_number = line.get_counter(SRC_LINE);

            gooda::afdo_stack stack;
            stack.count = line.get_counter(SRC_UNHALTED_CORE_CYCLES);
            stack.num_inst = 1; 

            gooda::afdo_pos position;
            position.func = function.name;
            position.file = function.file;
            position.line = line_number;
            position.discr = 1;

            stack.stack.push_back(position);

            function.stacks.push_back(std::move(stack));
        }
    }
}

} //End of anonymous namespace

void gooda::read_report(const gooda_report& report, gooda::afdo_data& data){
    for(std::size_t i = 0; i < report.functions(); ++i){
        auto& line = report.hotspot_function(i);
    
        gooda::afdo_function function;
        function.name = line.get_string(HS_FUNCTION_NAME);
        function.file = "unknown";
        function.total_count = line.get_counter(HS_UNHALTED_CORE_CYCLES);

        data.add_file_name(function.file);
        data.add_file_name(function.name);

        data.functions.push_back(function);
        
        read_asm_file(report, i, data);
        read_src_file(report, i, data);
    }

    //Note: No need to fill the working set because it is not used by GCC
}
