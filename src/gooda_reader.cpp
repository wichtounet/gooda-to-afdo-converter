//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>

#include "gooda_reader.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "likely.hpp"
#include "gooda_exception.hpp"

#define HOTSPOT_CSV "/function_hotspots.csv"
#define PROCESS_CSV "/process.csv"

#define ASM_FOLDER "/asm/"
#define ASM_CSV "_asm.csv"

#define SRC_FOLDER "/src/"
#define SRC_CSV "_src.csv"

/*!
 * \file gooda_reader.cpp
 * \brief Implementation of the reading and parsing of Gooda spreadsheets into the gooda::gooda_report data structure. 
 */

namespace {

/*!
 * \brief Parse a Gooda line into a vector of pair of iterators denoting the columns. 
 * \param line The string line
 * \param contents The vector to fill
 */
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
}

/*!
 * \brief Parse the headers of the given gooda_file
 *
 * Only the column names and the multipled information are extracted from the headers,
 * the other header lines are ignored. 
 *
 * \param file The file currently read
 * \param gooda_file The gooda_file to fill
 */
void parse_headers(std::ifstream& file, gooda::gooda_file& gooda_file){
    std::string line;

    //Introduction of the array
    std::getline(file, line);

    //Headers
    std::getline(file, line);
    
    //Parse the column names into the cache
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

    auto& multiplex_line = gooda_file.multiplex_line();
    multiplex_line.line() = line;

    parse_gooda_line(multiplex_line.line(), multiplex_line.contents());
    
    //Penalty
    std::getline(file, line);
    
    //Cycles
    std::getline(file, line);
}

/*!
 * \brief Open the given file_name into the given stream. 
 * \param file The ifstream to open.
 * \param file_name The path to the file.
 * \param must_exists If set to true and the file does not exists, throws an exception
 * \return true if the file exists, false otherwise
 */
bool open_file(std::ifstream& file, std::string file_name, bool must_exists){
    bool exists = gooda::exists(file_name);

    if(must_exists && !exists){
        throw gooda::gooda_exception("\"" + file_name + "\" does not exist");
    }

    if(exists){
        file.open (file_name, std::ios::in);

        if(!file.is_open()){
            throw gooda::gooda_exception("Unable to open \"" + file_name + "\"");
        }
    }

    return exists;
}

/*!
 * \brief Read a gooda file and fill the corresponding gooda_file
 * \param file The file to read,
 * \param gooda_file The gooda_file to fille.
 */
void read_gooda_file(std::ifstream& file, gooda::gooda_file& gooda_file){
    parse_headers(file, gooda_file);

    std::string line;

    //The first line
    std::getline(file, line);

    while(line.size() > 3){
        auto& gooda_line = gooda_file.new_line();
        gooda_line.line() = line;

        //Parse the contents of the line
    variables_map      
        ::parsed_options parsed_options;

asifsjfajfdisjfq        
        parse_gooda_line(gooda_line.line(), gooda_line.contents());

        //Next line
        std::getline(file, line);
    }
}

/*!
 * \brief Read the list of the processes. 
 * \param directory The spreadsheets directory. 
 * \param report The gooda_report to fill.
 */
void read_processes(const std::string& directory, gooda::gooda_report& report){
    //Open the file
    std::ifstream process_file;
    open_file(process_file, directory + PROCESS_CSV, true);

    //Read and parse the gooda file
    read_gooda_file(process_file, report.get_process_file());

    log::emit<log::Debug>() << "Found " << report.processes() << " processes" << log::endl;
}

/*!
 * \brief Read the hotspot function list
 * \param directory The spreadsheets directory. 
 * \param report The gooda_report to fill.
 */
void read_hotspot(const std::string& directory, gooda::gooda_report& report){
    //Open the file
    std::ifstream hotspot_file;
    open_file(hotspot_file, directory + HOTSPOT_CSV, true);

    //Read and parse the gooda file
    read_gooda_file(hotspot_file, report.get_hotspot_file());

    log::emit<log::Debug>() << "Found " << report.functions() << " hotspot functions" << log::endl;
}

/*!
 * \brief Read the assembly view file for the given function. 
 * \param directory The spreadsheets directory. 
 * \param i The index of the function
 * \param report The gooda_report to fill.
 */
void read_asm_file(const std::string& directory, std::size_t i, gooda::gooda_report& report){
    std::ifstream asm_file;

    //Try to open the file
    if(open_file(asm_file, directory + ASM_FOLDER + std::to_string(i) + ASM_CSV, false)){
        //Read and parse the gooda file
        read_gooda_file(asm_file, report.asm_file(i));
    }
}

/*!
 * \brief Read the source view file for the given function. 
 * \param directory The spreadsheets directory. 
 * \param i The index of the function
 * \param report The gooda_report to fill.
 */
void read_src_file(const std::string& directory, std::size_t i, gooda::gooda_report& report){
    std::ifstream src_file;
    
    //Try to open the file
    if(open_file(src_file, directory + SRC_FOLDER + std::to_string(i) + SRC_CSV, false)){
        //Read and parse the gooda file
        read_gooda_file(src_file, report.src_file(i));
    }
}

} //end of anonymous namespace

gooda::gooda_report gooda::read_spreadsheets(const std::string& directory){
    log::emit<log::Debug>() << "Import spreadsheets from " << directory << log::endl;

    gooda::gooda_report report;

    //Read the process and hotspot views
    read_processes(directory, report);
    read_hotspot(directory, report);

    //Read the assembly and source views of each hotspot function
    for(std::size_t i = 0; i < report.functions(); ++i){
        read_asm_file(directory, i, report);
        read_src_file(directory, i, report);
    }

    return report;
}
