#include <cassert>

#include "afdo_data.hpp"

gooda::afdo_data::afdo_data() : working_set(WS_SIZE + 1) {
    //Nothing to init
}

gcov_unsigned_t gooda::afdo_data::get_file_index(const std::string& file) const {
    assert(file_index.count(file));

    return file_index.at(file);
}

void gooda::afdo_data::add_file_name(const std::string& file){
    if(file_index.find(file) == file_index.end()){
        file_index[file] = file_names.size();
        file_names.push_back(file);
    }
}
