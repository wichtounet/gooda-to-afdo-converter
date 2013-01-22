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

inline void parse_options(gooda::options& options, std::string param1, std::string folder){
    std::string folder_arg = "--folder=" + folder;

    const char* argv[6];
    argv[0] = "./bin/test";
    argv[1] = "--log=3";
    argv[2] = "--discriminators";
    argv[3] = param1.c_str();
    argv[4] = folder_arg.c_str();
    argv[5] = "";

    BOOST_REQUIRE_EQUAL (options.parse(6, argv), 0);
    BOOST_REQUIRE_EQUAL (options.notify(), 0);
}

BOOST_AUTO_TEST_SUITE(MainSuite)

BOOST_AUTO_TEST_CASE( simple_ucc ){
    gooda::options options;
    parse_options(options, "--nows", "tests/cases/simple/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);

    auto& function = data.functions.front();

    BOOST_CHECK_EQUAL(function.name, "main");
    BOOST_CHECK_EQUAL(function.file, "simple.cpp");
    BOOST_CHECK_EQUAL(function.total_count, 4021);
    BOOST_CHECK_EQUAL(function.entry_count, 0);
}

BOOST_AUTO_TEST_SUITE_END()
