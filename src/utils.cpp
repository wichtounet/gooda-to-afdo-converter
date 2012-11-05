#include <sys/stat.h>

#include "utils.hpp"

bool converter::exists(const std::string& file){
    struct stat buf;
    
    return stat(file.c_str(), &buf) != -1;
}

bool converter::is_directory(const std::string& file){
    struct stat st;
    lstat(file.c_str(), &st);

    return S_ISDIR(st.st_mode);
}
