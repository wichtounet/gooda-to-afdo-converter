#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "reader.hpp"

#define FUNCTION_NAME 2
#define UNHALTED_CORE_CYCLES 7
#define INSTRUCTION_RETIRED 9

namespace {

typedef std::string::const_iterator string_iter;
typedef boost::iterator_range<string_iter> string_view;

void remove_quotes(std::string& str){
    if(str.size() >= 2 && str[0] == '"' && str[str.length() - 1] == '"'){
        str = str.substr(1, str.length() - 2);
    }
}

std::vector<string_view> parse_gooda_hotspot_line(const std::string& line){
    //Keep only the interesting part
    std::string contents_line = line.substr(2, line.size() - 5);

    std::vector<string_view> contents;

    boost::split(contents, contents_line, [](char c){return c == ',';});

    return contents;
}

std::string get_string(std::vector<string_view>& contents, int index){
    auto& item = contents[index];
    
    std::string value(item.begin(), item.end());

    boost::trim(value);
    remove_quotes(value);

    return value;
}

gcov_type get_counter(std::vector<string_view>& contents, int index){
    auto& item = contents[index];
    
    std::string value(item.begin(), item.end());

    boost::trim(value);

    return boost::lexical_cast<gcov_type>(value);
}

} //End of anonymous namespace

bool converter::read_spreadsheets(const std::string& directory, converter::Data& data){
    std::cout << "Import spreadsheets from " << directory << std::endl;

    std::string hotspot_file_name = directory + "/function_hotspots.csv";

    std::ifstream hotspot_file;
    hotspot_file.open (hotspot_file_name, std::ios::in);

    if(!hotspot_file.is_open()){
        std::cout << "Unable to open \"" << hotspot_file_name << "\"" << std::endl;
        return false;
    }

    std::string line;

    //Introduction of the array
    std::getline(hotspot_file, line);

    //Headers
    std::getline(hotspot_file, line);
    
    //Events
    std::getline(hotspot_file, line);
    
    //MSR Programming
    std::getline(hotspot_file, line);
    
    //Period
    std::getline(hotspot_file, line);
    
    //Multiplex
    std::getline(hotspot_file, line);
    
    //Penalty
    std::getline(hotspot_file, line);
    
    //Cycles
    std::getline(hotspot_file, line);

    //The first hotspot line
    std::getline(hotspot_file, line);
    
    while (!line.empty()) {
        auto contents = parse_gooda_hotspot_line(line);

        converter::Function function;
        function.name = get_string(contents, FUNCTION_NAME);
        function.file = "unknown";
        function.total_count = get_counter(contents, UNHALTED_CORE_CYCLES);
        function.entry_count = 0;

        data.add_file_name(function.file);

        data.functions.push_back(function);

        std::getline(hotspot_file, line);
    }

    std::cout << "Found " << data.functions.size() << " hotspot functions" << std::endl;

    return true;
}
