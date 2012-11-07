#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "gooda_reader.hpp"
#include "utils.hpp"

#define HOTSPOT_CSV "/function_hotspots.csv"

#define ASM_FOLDER "/asm/"
#define ASM_CSV "_asm.csv"

#define SRC_FOLDER "/src/"
#define SRC_CSV "_src.csv"

namespace {

void skip_headers(std::ifstream& file){
    std::string line;

    //Introduction of the array
    std::getline(file, line);

    //Headers
    std::getline(file, line);
    
    //Events
    std::getline(file, line);
    
    //MSR Programming
    std::getline(file, line);
    
    //Period
    std::getline(file, line);
    
    //Multiplex
    std::getline(file, line);
    
    //Penalty
    std::getline(file, line);
    
    //Cycles
    std::getline(file, line);
}

void parse_gooda_line(std::string& line, std::vector<string_view>& contents){
    //Keep only the interesting part
    line = line.substr(2, line.size() - 5);

    boost::split(contents, line, [](char c){return c == ',';});
}

bool read_hotspot(const std::string& directory, converter::gooda_report& report){
    std::string hotspot_file_name = directory + HOTSPOT_CSV;

    std::ifstream hotspot_file;
    hotspot_file.open (hotspot_file_name, std::ios::in);

    if(!hotspot_file.is_open()){
        std::cout << "Unable to open \"" << hotspot_file_name << "\"" << std::endl;
        return false;
    }

    skip_headers(hotspot_file);

    std::string line;

    //The first hotspot line
    std::getline(hotspot_file, line);
    
    while(!line.empty()){
        converter::gooda_line hotspot_line;
        hotspot_line.line = line;

        //Parse the contents of the line
        parse_gooda_line(hotspot_line.line, hotspot_line.contents);

        //Next line
        std::getline(hotspot_file, line);
    }

    std::cout << "Found " << report.functions() << " hotspot functions" << std::endl;

    return true;
}

void read_asm_file(const std::string& directory, std::size_t i, converter::gooda_report& report){
    std::string asm_file_name = directory + ASM_FOLDER + std::to_string(i) + ASM_CSV;

    if(converter::exists(asm_file_name)){
        std::ifstream asm_file;
        asm_file.open (asm_file_name, std::ios::in);

        if(!asm_file.is_open()){
            std::cout << "Unable to open \"" << asm_file_name << "\"" << std::endl;
            return;
        }

        converter::gooda_file gooda_file = report.asm_file(i);

        skip_headers(asm_file);

        std::string line;

        //The first asm line
        std::getline(asm_file, line);

        while(line.size() > 3){
            converter::gooda_line asm_line;
            asm_line.line = line;

            //Parse the contents of the line
            parse_gooda_line(asm_line.line, asm_line.contents);

            gooda_file.lines.push_back(asm_line);

            //Next line
            std::getline(asm_file, line);
        }
    }
}

void read_src_file(const std::string& directory, std::size_t i, converter::gooda_report& report){
    std::string src_file_name = directory + SRC_FOLDER + std::to_string(i) + SRC_CSV;

    if(converter::exists(src_file_name)){
        std::ifstream src_file;
        src_file.open (src_file_name, std::ios::in);

        if(!src_file.is_open()){
            std::cout << "Unable to open \"" << src_file_name << "\"" << std::endl;
            return;
        }

        converter::gooda_file gooda_file = report.src_file(i);

        skip_headers(src_file);

        std::string line;

        //The first src line
        std::getline(src_file, line);

        while(line.size() > 3){
            converter::gooda_line src_line;
            src_line.line = line;

            //Parse the contents of the line
            parse_gooda_line(src_line.line, src_line.contents);

            gooda_file.lines.push_back(src_line);

            //Next line
            std::getline(src_file, line);
        }
    }
}

} //end of anonymous namespace

converter::gooda_report converter::read_spreadsheets(const std::string& directory){
    converter::gooda_report report;

    if(read_hotspot(directory, report)){
        for(std::size_t i = 0; i < report.functions(); ++i){
            read_asm_file(directory, i, report);
            read_src_file(directory, i, report);
        }
    }

    return report;
}
