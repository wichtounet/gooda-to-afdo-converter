#include <cassert>

#include "data.hpp"

gcov_unsigned_t converter::Data::get_file_index(const std::string& file){
    assert(file_index.count(file));

    return file_index[file];
}

void converter::Data::add_file_name(const std::string& file){
    if(file_index.find(file) == file_index.end()){
        file_index[file] = file_names.size();
        file_names.push_back(file);
    }
}
