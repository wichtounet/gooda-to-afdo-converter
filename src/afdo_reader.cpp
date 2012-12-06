//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "afdo_reader.hpp"
#include "gcov_file.hpp"

void gooda::read_afdo(const std::string& afdo_file, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    gooda::gcov_file gcov_file;

    if(!gcov_file.open_for_read(afdo_file)){
        std::cout << "Cannot open file for reading" << std::endl;
        return;
    }

}
