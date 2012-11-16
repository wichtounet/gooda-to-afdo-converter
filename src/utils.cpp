#include <sstream>
#include <iostream>
#include <fstream>

#include <sys/stat.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "utils.hpp"

bool gooda::exists(const std::string& file){
    struct stat buf;
    
    return stat(file.c_str(), &buf) != -1;
}

bool gooda::is_directory(const std::string& file){
    struct stat st;
    lstat(file.c_str(), &st);

    return S_ISDIR(st.st_mode);
}

std::string gooda::exec_command(const std::string& command) {
    std::stringstream output;

    char buffer[1024];

    /*FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        output << buffer;
    }

    pclose(stream);*/

    system(command.c_str());

    return output.str();
}

int gooda::processor_model(){
    std::ifstream cpuinfo_file;
    cpuinfo_file.open ("/proc/cpuinfo", std::ios::in);

    if(!cpuinfo_file.is_open()){
        std::cout << "Unable to open \"/proc/cpuinfo\"" << std::endl;
        return -1;
    }

    std::string line;

    while(!cpuinfo_file.eof()){
        std::getline(cpuinfo_file, line);

        if(boost::starts_with(line, "model\t")){
            std::vector<std::string> parts;
            boost::split(parts, line, [](char a){return a == ':';});
            
            std::string model_str = parts[1];
            boost::trim(model_str);
            
            return boost::lexical_cast<int>(model_str);
        }
    }

    return -1;
}
