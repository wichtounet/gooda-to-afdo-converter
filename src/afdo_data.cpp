//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "assert.hpp"
#include "afdo_data.hpp"

gcov_unsigned_t gooda::afdo_data::get_file_index(const std::string& file) const {
    assert(file_index.count(file) > 0);

    return file_index.at(file);
}

const std::string& gooda::afdo_data::file_name(std::size_t i) const {
    gooda_assert(i < file_names.size(), "The index is not contained in the file name table");

    return file_names.at(i);
}

void gooda::afdo_data::add_file_name(const std::string& file){
    if(file_index.find(file) == file_index.end()){
        file_index[file] = file_names.size();
        file_names.push_back(file);
    }
}
