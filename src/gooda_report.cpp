#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "gooda_report.hpp"

std::string converter::gooda_line::get_string(std::size_t index) const {
    auto& item = contents[index];

    std::string v(item.begin(), item.end());
    std::string value = v;

    boost::trim(value);

    return value;
}

unsigned long converter::gooda_line::get_counter(std::size_t index) const {
    auto& item = contents[index];
    
    //std::cout << index << ":" << item << std::endl;

    std::string v(item.begin(), item.end());
    std::string value = v;

    boost::trim(value);
    
    /*
    for(std::size_t j = 0; j < contents.size(); ++j){
        std::cout << j << ":" << contents[j] << std::endl;
    }
    
    std::cout << index << ":" << value << std::endl;
    */

    return boost::lexical_cast<unsigned long>(value);
}

converter::gooda_line& converter::gooda_file::new_line(){
    int i = lines.size();

    lines.resize(i + 1);

    return lines[i];
}

std::size_t converter::gooda_file::size() const {
    return lines.size();
}

converter::gooda_line& converter::gooda_file::line(std::size_t i){
    return lines[i];
}

const converter::gooda_line& converter::gooda_file::line(std::size_t i) const {
    return lines[i];
}

converter::gooda_file::iterator converter::gooda_file::begin(){
    return lines.begin();
}

converter::gooda_file::iterator converter::gooda_file::end(){
    return lines.end();
}

converter::gooda_file::const_iterator converter::gooda_file::begin() const {
    return lines.cbegin();
}

converter::gooda_file::const_iterator converter::gooda_file::end() const {
    return lines.cend();
}

std::size_t converter::gooda_report::functions() const {
    return hotspot_file.size();
}
        
converter::gooda_file& converter::gooda_report::src_file(std::size_t i){
    return src_files[i];
}

converter::gooda_file& converter::gooda_report::asm_file(std::size_t i){
    return asm_files[i];
}

const converter::gooda_file& converter::gooda_report::asm_file(std::size_t i) const {
    return asm_files.at(i);
}

const converter::gooda_file& converter::gooda_report::src_file(std::size_t i) const {
    return src_files.at(i);
}

bool converter::gooda_report::has_src_file(std::size_t i) const {
    return src_files.find(i) != src_files.end();
}

bool converter::gooda_report::has_asm_file(std::size_t i) const {
    return asm_files.find(i) != asm_files.end();
}

converter::gooda_line& converter::gooda_report::new_hotspot_function(){
    return hotspot_file.new_line();
}

const converter::gooda_line& converter::gooda_report::hotspot_function(std::size_t i) const {
    return hotspot_file.line(i);
}
