#include "gooda_report.hpp"

std::size_t converter::gooda_report::functions() const {
    return hotspot_functions.size();
}
        
void converter::gooda_report::add_hotspot_function(gooda_line&& line){
    hotspot_functions.push_back(line);
}
