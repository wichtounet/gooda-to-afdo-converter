#include "gooda_report.hpp"

std::size_t converter::gooda_report::functions() const {
    return hotspot_file.lines.size();
}
        
void converter::gooda_report::add_hotspot_function(gooda_line&& line){
    hotspot_file.lines.push_back(line);
}
        
converter::gooda_file& converter::gooda_report::src_file(std::size_t i){
    return src_files[i];
}

converter::gooda_file& converter::gooda_report::asm_file(std::size_t i){
    return asm_files[i];
}
