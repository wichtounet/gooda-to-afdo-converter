//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <boost/algorithm/string.hpp>

#include "gooda_reader.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "likely.hpp"

#define HOTSPOT_CSV "/function_hotspots.csv"
#define PROCESS_CSV "/process.csv"

#define ASM_FOLDER "/asm/"
#define ASM_CSV "_asm.csv"

#define SRC_FOLDER "/src/"
#define SRC_CSV "_src.csv"

namespace {

void parse_gooda_line(std::string& line, std::vector<string_view>& contents){
    //Keep only the interesting part
    line = line.substr(2, line.size() - 5);

    auto it = line.begin();
    auto end = line.end();

    unsigned long length = 0;

    while(it != end){
        auto c = *it;

        if(unlikely(c == ',')){
            contents.emplace_back(it - length, it);
            length = 0;
        } else if(unlikely(c == '\"')){
            length = 0;

            do {
                ++it;
                c = *it;
                ++length;

                if(unlikely(c == '\\')){
                    ++it;
                    ++length;
                }
            } while(c != '\"');

            contents.emplace_back(it - length + 1, it);
            
            while(c != ',' && it != end){
                ++it;
                c = *it;
            }

            length = 0;
        } else {
            ++length;
        }
        
        if(likely(it != end)){
            ++it;
        }
    }
    
    if(length > 0){
        contents.emplace_back(it - length, it);
    }

    /*std::cout << line << std::endl;
    
    for(std::size_t j = 0; j < contents.size(); ++j){
        std::cout << j << ":" << contents[j] << std::endl;
    }*/
}

void skip_headers(std::ifstream& file, gooda::gooda_file& gooda_file){
    std::string line;

    //Introduction of the array
    std::getline(file, line);

    //Headers
    std::getline(file, line);
    
    std::vector<string_view> headers; 
    parse_gooda_line(line, headers);

    for(std::size_t i = 0; i < headers.size(); ++i){
        auto& header = headers[i];

        std::string v(header.begin(), header.end());
        boost::trim(v);

        gooda_file.column(v) = i;
    }
    
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

bool read_processes(const std::string& directory, gooda::gooda_report& report){
    std::string process_file_name = directory + PROCESS_CSV;

    std::ifstream process_file;
    process_file.open (process_file_name, std::ios::in);

    if(!process_file.is_open()){
        std::cout << "Unable to open \"" << process_file_name << "\"" << std::endl;
        return false;
    }

    skip_headers(process_file, report.get_process_file());

    std::string line;

    //The first process line
    std::getline(process_file, line);

    int i = 0;
    
    while(line.size() > 2){
        auto& process_line = report.new_process();
        process_line.line() = line;

        //Parse the contents of the line
        parse_gooda_line(process_line.line(), process_line.contents());

        //Next line
        std::getline(process_file, line);

        ++i;
    }

    log::emit<log::Debug>() << "Found " << report.processes() << " processes" << log::endl;

    return true;
}

bool read_hotspot(const std::string& directory, gooda::gooda_report& report){
    std::string hotspot_file_name = directory + HOTSPOT_CSV;

    std::ifstream hotspot_file;
    hotspot_file.open (hotspot_file_name, std::ios::in);

    if(!hotspot_file.is_open()){
        std::cout << "Unable to open \"" << hotspot_file_name << "\"" << std::endl;
        return false;
    }

    skip_headers(hotspot_file, report.get_hotspot_file());

    std::string line;

    //The first hotspot line
    std::getline(hotspot_file, line);

    int i = 0;
    
    while(!line.empty()){
        auto& hotspot_line = report.new_hotspot_function();
        hotspot_line.line() = line;

        //Parse the contents of the line
        parse_gooda_line(hotspot_line.line(), hotspot_line.contents());

        //Next line
        std::getline(hotspot_file, line);

        ++i;
    }

    log::emit<log::Debug>() << "Found " << report.functions() << " hotspot functions" << log::endl;

    return true;
}

void read_asm_file(const std::string& directory, std::size_t i, gooda::gooda_report& report){
    std::string asm_file_name = directory + ASM_FOLDER + std::to_string(i) + ASM_CSV;

    if(gooda::exists(asm_file_name)){
        std::ifstream asm_file;
        asm_file.open (asm_file_name, std::ios::in);

        if(!asm_file.is_open()){
            std::cout << "Unable to open \"" << asm_file_name << "\"" << std::endl;
            return;
        }

        auto& gooda_file = report.asm_file(i);

        skip_headers(asm_file, gooda_file);

        std::string line;

        //The first asm line
        std::getline(asm_file, line);

        while(line.size() > 3){
            auto& asm_line = gooda_file.new_line();
            asm_line.line() = line;

            //Parse the contents of the line
            parse_gooda_line(asm_line.line(), asm_line.contents());

            //Next line
            std::getline(asm_file, line);
        }
    }
}

void read_src_file(const std::string& directory, std::size_t i, gooda::gooda_report& report){
    std::string src_file_name = directory + SRC_FOLDER + std::to_string(i) + SRC_CSV;

    if(gooda::exists(src_file_name)){
        std::ifstream src_file;
        src_file.open (src_file_name, std::ios::in);

        if(!src_file.is_open()){
            std::cout << "Unable to open \"" << src_file_name << "\"" << std::endl;
            return;
        }

        auto& gooda_file = report.src_file(i);

        skip_headers(src_file, gooda_file);

        std::string line;

        //The first src line
        std::getline(src_file, line);

        while(line.size() > 3){
            auto& src_line = gooda_file.new_line();
            src_line.line() = line;

            //Parse the contents of the line
            parse_gooda_line(src_line.line(), src_line.contents());

            //Next line
            std::getline(src_file, line);
        }
    }
}

} //end of anonymous namespace

gooda::gooda_report gooda::read_spreadsheets(const std::string& directory){
    log::emit<log::Debug>() << "Import spreadsheets from " << directory << log::endl;

    gooda::gooda_report report;

    read_processes(directory, report);

    if(read_hotspot(directory, report)){
        for(std::size_t i = 0; i < report.functions(); ++i){
            read_asm_file(directory, i, report);
            read_src_file(directory, i, report);
        }
    }

    return report;
}
