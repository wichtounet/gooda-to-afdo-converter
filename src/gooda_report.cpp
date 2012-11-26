#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "gooda_report.hpp"

std::string gooda::gooda_line::get_string(std::size_t index) const {
    auto& item = contents[index];

    std::string v(item.begin(), item.end());
    std::string value = v;

    boost::trim(value);

    return value;
}

unsigned long gooda::gooda_line::get_counter(std::size_t index) const {
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

gooda::gooda_line& gooda::gooda_file::new_line(){
    int i = lines.size();

    lines.resize(i + 1);

    return lines[i];
}

std::size_t gooda::gooda_file::size() const {
    return lines.size();
}

gooda::gooda_line& gooda::gooda_file::line(std::size_t i){
    return lines[i];
}

const gooda::gooda_line& gooda::gooda_file::line(std::size_t i) const {
    return lines[i];
}

gooda::gooda_file::iterator gooda::gooda_file::begin(){
    return lines.begin();
}

gooda::gooda_file::iterator gooda::gooda_file::end(){
    return lines.end();
}

gooda::gooda_file::const_iterator gooda::gooda_file::begin() const {
    return lines.cbegin();
}

gooda::gooda_file::const_iterator gooda::gooda_file::end() const {
    return lines.cend();
}

unsigned int& gooda::gooda_file::column(const std::string& column){
    return columns[column];
}

const unsigned int& gooda::gooda_file::column(const std::string& column) const {
    return columns.at(column);
}

std::size_t gooda::gooda_report::functions() const {
    return hotspot_file.size();
}

std::size_t gooda::gooda_report::processes() const {
    return process_file.size();
}
        
gooda::gooda_file& gooda::gooda_report::src_file(std::size_t i){
    return src_files[i];
}

gooda::gooda_file& gooda::gooda_report::asm_file(std::size_t i){
    return asm_files[i];
}

const gooda::gooda_file& gooda::gooda_report::asm_file(std::size_t i) const {
    return asm_files.at(i);
}

const gooda::gooda_file& gooda::gooda_report::src_file(std::size_t i) const {
    return src_files.at(i);
}

bool gooda::gooda_report::has_src_file(std::size_t i) const {
    return src_files.find(i) != src_files.end();
}

bool gooda::gooda_report::has_asm_file(std::size_t i) const {
    return asm_files.find(i) != asm_files.end();
}

gooda::gooda_line& gooda::gooda_report::new_process(){
    return process_file.new_line();
}

const gooda::gooda_line& gooda::gooda_report::process(std::size_t i) const {
    return process_file.line(i);
}

gooda::gooda_line& gooda::gooda_report::new_hotspot_function(){
    return hotspot_file.new_line();
}

const gooda::gooda_line& gooda::gooda_report::hotspot_function(std::size_t i) const {
    return hotspot_file.line(i);
}

gooda::gooda_file& gooda::gooda_report::get_hotspot_file(){
    return hotspot_file;
}

gooda::gooda_file& gooda::gooda_report::get_process_file(){
    return process_file;
}

const gooda::gooda_file& gooda::gooda_report::get_hotspot_file() const {
    return hotspot_file;
}

const gooda::gooda_file& gooda::gooda_report::get_process_file() const {
    return process_file;
}
