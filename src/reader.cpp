#include <iostream>
#include <fstream>
#include <sstream>

#include "reader.hpp"

void trim(std::string& str){
    std::stringstream trimmer;
    trimmer << str;
    str.clear();
    trimmer >> str;
}

void remove_quotes(std::string& str){
    if(str.size() < 2){
        return;
    }

    if(str[0] == '"' && str[str.length() - 1] == '"'){
        str = str.substr(1, str.length() - 2);
    }
}

void split(const std::string& line, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(line);
    std::string item;

    while(std::getline(ss, item, delim)) {
        trim(item);
        remove_quotes(item);
        elems.push_back(item);
    }
}

std::vector<std::string> parse_gooda_hotspot_line(const std::string& line){
    //Keep only the interesting part
    std::string contents_line = line.substr(2, line.size() - 5);

    std::vector<std::string> contents;

    split(contents_line, ',', contents);

    return contents;
}

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
    
    while (line.substr(0, 3) != "[,\"") {
        if(!line.empty()){
            auto contents = parse_gooda_hotspot_line(line);

            std::cout << "Hotspot function " << contents[2] << std::endl;

            Function function;
            function.name = contents[2];
            function.file = "unknown";

            data.add_file_name(function.file);

            data.functions.push_back(function);
        }

        std::getline(hotspot_file, line);
    }

    std::cout << "Found " << data.functions.size() << " hotspot functions" << std::endl;

    return true;
}
