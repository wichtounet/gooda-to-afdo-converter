#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "reader.hpp"
#include "utils.hpp"

//Hotspot indices
#define HS_FUNCTION_NAME 2
#define HS_UNHALTED_CORE_CYCLES 7
#define HS_INSTRUCTION_RETIRED 9

//asm indices
#define ASM_DISASSEMBLY 6
#define ASM_UNHALTED_CORE_CYCLES 7

#define HOTSPOT_CSV "/function_hotspots.csv"

#define ASM_FOLDER "/asm/"
#define ASM_CSV "_asm.csv"

#define SRC_FOLDER "/src/"
#define SRC_CSV "_src.csv"

namespace {

typedef std::string::const_iterator string_iter;
typedef boost::iterator_range<string_iter> string_view;

void remove_quotes(std::string& str){
    if(str.size() >= 2 && str[0] == '"' && str[str.length() - 1] == '"'){
        str = str.substr(1, str.length() - 2);
    }
}

std::vector<string_view> parse_gooda_line(const std::string& line){
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

bool read_hotspot(const std::string& directory, converter::Data& data){
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
        auto contents = parse_gooda_line(line);

        converter::Function function;
        function.name = get_string(contents, HS_FUNCTION_NAME);
        function.file = "unknown";
        function.total_count = get_counter(contents, HS_UNHALTED_CORE_CYCLES);

        data.add_file_name(function.file);

        data.functions.push_back(function);

        std::getline(hotspot_file, line);
    }

    std::cout << "Found " << data.functions.size() << " hotspot functions" << std::endl;

    return true;
}

} //End of anonymous namespace

bool converter::read_spreadsheets(const std::string& directory, converter::Data& data){
    std::cout << "Import spreadsheets from " << directory << std::endl;

    //Import the hotspot functions
    if(!read_hotspot(directory, data)){
        return false;
    }

    for(std::size_t i = 0; i < data.functions.size(); ++i){
        auto& function = data.functions[i];

        std::string asm_file_name = directory + ASM_FOLDER + std::to_string(i) + ASM_CSV;

        if(converter::exists(asm_file_name)){
            std::ifstream asm_file;
            asm_file.open (asm_file_name, std::ios::in);

            if(!asm_file.is_open()){
                std::cout << "Unable to open \"" << asm_file_name << "\"" << std::endl;
                return false;
            }

            skip_headers(asm_file);
            
            std::string line;

            //The first hotspot line
            std::getline(asm_file, line);

            while(line.size() > 3){
                auto contents = parse_gooda_line(line);

                auto disassembly = get_string(contents, ASM_DISASSEMBLY);

                //Get the entry basic block
                if(boost::starts_with(disassembly, " Basic Block 1 <")){
                    function.entry_count = get_counter(contents, ASM_UNHALTED_CORE_CYCLES);
                }

                std::getline(asm_file, line);
            }

            std::cout << function.name << ":" << function.total_count << ":" << function.entry_count << std::endl;
        }
    }

    return true;
}
