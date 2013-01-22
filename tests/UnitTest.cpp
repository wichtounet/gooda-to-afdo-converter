//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ConverterTestSuites
#include <boost/test/unit_test.hpp>
#include <boost/test/detail/unit_test_parameters.hpp>

#include "Options.hpp"
#include "gooda_reader.hpp"
#include "converter.hpp"

inline void parse_options(gooda::options& options, std::string param1, std::string param2, std::string param3){
    const char* argv[5];
    argv[0] = "./bin/test";
    argv[1] = param1.c_str();
    argv[2] = param2.c_str();
    argv[3] = param3.c_str();

    BOOST_REQUIRE (options.parse(4, argv) == 0);
    BOOST_REQUIRE (options.notify() == 0);
}

BOOST_AUTO_TEST_SUITE(MainSuite)

BOOST_AUTO_TEST_CASE( args_lbr ){
    gooda::options options;
    parse_options(options, "--quiet", "--discriminators", "--lbr");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple-lbr/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);
}

BOOST_AUTO_TEST_SUITE_END()
