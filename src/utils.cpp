//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file utils.cpp
 * \brief Implementation of the various utility functions. 
 */

#include <sstream>
#include <iostream>
#include <fstream>

#include <sys/stat.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "utils.hpp"
#include "gooda_exception.hpp"

bool gooda::exists(const std::string& file){
    struct stat buf;
    
    return stat(file.c_str(), &buf) != -1;
}

bool gooda::is_directory(const std::string& file){
    struct stat st;
    lstat(file.c_str(), &st);

    return S_ISDIR(st.st_mode);
}

int gooda::exec_command(const std::string& command) {
    return system(command.c_str());
}

std::string gooda::exec_command_result(const std::string& command){
    FILE* stream = popen(command.c_str(), "r");

    if(!stream){
        throw gooda::gooda_exception("Unable to execute \"" + command + "\": Null pipe");
    }
    
    std::string result;
    char buffer[1024];

    while(!feof(stream)) {
        if(fgets(buffer, 1024, stream) != NULL){
            result += buffer;
        }
    }

    pclose(stream);
    
    return result;
}

int gooda::processor_model(){
    std::ifstream cpuinfo_file;
    cpuinfo_file.open ("/proc/cpuinfo", std::ios::in);

    if(!cpuinfo_file.is_open()){
        throw gooda::gooda_exception("Unable to read /proc/cpuinfo");
    }

    std::string line;

    while(!cpuinfo_file.eof()){
        std::getline(cpuinfo_file, line);

        if(boost::starts_with(line, "model\t")){
            std::vector<std::string> parts;
            boost::split(parts, line, [](char a){return a == ':';});
            
            std::string model_str = parts.at(1);
            boost::trim(model_str);
            
            return boost::lexical_cast<int>(model_str);
        }
    }

    throw gooda::gooda_exception("Uhandled /proc/cpuinfo format");
}
