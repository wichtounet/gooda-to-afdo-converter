#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include "reader.hpp"
#include "utils.hpp"

namespace {

void read_asm_file(const converter::gooda_report& report, std::size_t i, converter::Data& data){
    if(report.has_asm_file(i)){
        auto& function = data.functions[i];
        auto& file = report.asm_file(i);

        for(auto& line : file){
            auto disassembly = line.get_string(ASM_DISASSEMBLY);
            
            //Get the entry basic block
            if(boost::starts_with(disassembly, " Basic Block 1 <")){
                function.entry_count = line.get_counter(ASM_UNHALTED_CORE_CYCLES);

                break;
            }
        }

        std::cout << function.name << ":" << function.total_count << ":" << function.entry_count << std::endl;
    }
}

void read_src_file(const converter::gooda_report& report, std::size_t i, converter::Data& data){
    if(report.has_src_file(i)){
        auto& function = data.functions[i];

        auto& file = report.src_file(i);

        for(auto& line : file){
            auto line_number = line.get_counter(SRC_LINE);

            std::cout << line_number << std::endl;
        }
    }
}

} //End of anonymous namespace

void converter::read_report(const gooda_report& report, converter::Data& data){
    for(std::size_t i = 0; i < report.functions(); ++i){
        auto& line = report.hotspot_function(i);

        converter::Function function;
        function.name = line.get_string(HS_FUNCTION_NAME);
        function.file = "unknown";
        function.total_count = line.get_counter(HS_UNHALTED_CORE_CYCLES);

        data.add_file_name(function.file);

        data.functions.push_back(function);
        
        read_asm_file(report, i, data);
        read_src_file(report, i, data);
    }
}
