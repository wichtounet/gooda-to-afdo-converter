#include <sstream>

#include <sys/stat.h>

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

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        output << buffer;
    }

    pclose(stream);

    return output.str();
}
