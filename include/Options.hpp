//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_OPTIONS_HPP
#define GOODA_OPTIONS_HPP

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
    
namespace po = boost::program_options;

namespace gooda {

struct options {
    po::options_description description;
    po::variables_map vm;
    po::parsed_options parsed_options;

    options() : description("converter [options] spreadsheets_directory"), parsed_options(&description){
        //Nothing to init
    }
    
    int parse(int argc, const char **argv);
    int notify();
};

} //end of namespace gooda

#endif
