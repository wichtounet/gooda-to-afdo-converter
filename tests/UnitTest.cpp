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
    argv[1] = "--quiet";
    argv[2] = "--discriminators";
    argv[3] = param1.c_str();
    argv[4] = folder_arg.c_str();
    argv[5] = "";

    BOOST_REQUIRE_EQUAL (options.parse(6, argv), 0);
    BOOST_REQUIRE_EQUAL (options.notify(), 0);
}

BOOST_AUTO_TEST_SUITE(MainSuite)

void check_contains_stack(const gooda::afdo_function& function, std::size_t count, std::pair<unsigned int, unsigned int> pos){
    bool found = false;

    for(auto& stack : function.stacks){
        if(stack.stack.size() == 1){
            auto& pos1 = stack.stack.front();

            if(pos1.line == pos.first && pos1.discriminator == pos.second){
                BOOST_CHECK(!found);

                BOOST_CHECK_EQUAL(stack.count, count);
                BOOST_CHECK_EQUAL(pos1.file, function.file);
                BOOST_CHECK_EQUAL(pos1.func, function.name);

                BOOST_CHECK(stack.num_inst > 0);

                found = true;
            }
        }
    }

    BOOST_CHECK(found);
}

void check_contains_inline_stack(const gooda::afdo_function& function, unsigned int line1, unsigned int line2, std::string file, std::string inlined, std::size_t count){
    bool found = false;

    for(auto& stack : function.stacks){
        if(stack.stack.size() == 2){
            auto& pos1 = stack.stack.front();
            auto& pos2 = stack.stack.back();

            if(pos1.line == line1 && pos2.line == line2 && pos1.discriminator == 0 && pos2.discriminator == 0 && pos2.file == file){
                BOOST_CHECK(!found);

                BOOST_CHECK_EQUAL(pos2.func, inlined);
                BOOST_CHECK_EQUAL(stack.count, count);
                BOOST_CHECK_EQUAL(pos1.file, function.file);
                BOOST_CHECK_EQUAL(pos1.func, function.name);

                BOOST_CHECK(stack.num_inst > 0);

                found = true;
            }
        }
    }

    BOOST_CHECK(found);
}

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

    //Verify function properties
    BOOST_CHECK_EQUAL(function.name, "main");
    BOOST_CHECK_EQUAL(function.file, "simple.cpp");
    BOOST_CHECK_EQUAL(function.total_count, 3971);
    BOOST_CHECK_EQUAL(function.entry_count, 0);

    //Basic Block 3
    check_contains_stack(function, 11, {22, 2});

    //Basic Block 7 (No discriminator here, should be the sum)
    check_contains_stack(function, 329, {20, 0});

    //Basic Block 8 (contains inlined functions)
    check_contains_inline_stack(function, 26, 10, "simple.cpp", "compute_sum", 787);
    check_contains_inline_stack(function, 26, 11, "simple.cpp", "compute_sum", 2817);

    //Basic Block 11 (contains function inlined from standard library)
    check_contains_inline_stack(function, 28, 111, "ostream", "_ZNSolsEPFRSoS_E", 0);
}

BOOST_AUTO_TEST_CASE( simple_lbr ){
    gooda::options options;
    parse_options(options, "--lbr", "tests/cases/simple/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple/lbr/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);

    auto& function = data.functions.front();

    //Verify function properties
    BOOST_CHECK_EQUAL(function.name, "main");
    BOOST_CHECK_EQUAL(function.file, "simple.cpp");
    BOOST_CHECK_EQUAL(function.total_count, 939390);
    BOOST_CHECK_EQUAL(function.entry_count, 0);

    //Basic Block 3
    check_contains_stack(function, 360, {22, 2});
    check_contains_stack(function, 360, {23, 2});
    
    //Basic Block 4 (Verify that the lines with different discriminators are different)
    check_contains_stack(function, 0, {22, 0});

    //Basic Block 7
    check_contains_stack(function, 93795, {20, 0});

    //Basic Block 8 (contains inlined functions)
    check_contains_inline_stack(function, 26, 10, "simple.cpp", "compute_sum", 93795);
    check_contains_inline_stack(function, 26, 11, "simple.cpp", "compute_sum", 93795);

    //Basic Block 11 (contains function inlined from standard library)
    check_contains_inline_stack(function, 28, 111, "ostream", "_ZNSolsEPFRSoS_E", 0);
    check_contains_inline_stack(function, 28, 165, "ostream", "_ZNSolsEl", 0);
}

BOOST_AUTO_TEST_CASE( simple_c_ucc ){
    gooda::options options;
    parse_options(options, "--nows", "tests/cases/simple-c/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple-c/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);

    auto& function = data.functions.front();

    //Verify function properties
    BOOST_CHECK_EQUAL(function.name, "main");
    BOOST_CHECK_EQUAL(function.file, "simple.c");
    BOOST_CHECK_EQUAL(function.total_count, 4079);
    BOOST_CHECK_EQUAL(function.entry_count, 0);

    //Basic Block 3
    check_contains_stack(function, 91, {24, 2});
    check_contains_stack(function, 0, {23, 0});
    check_contains_stack(function, 0, {23, 2});

    //Basic Block 7 (No discriminator here, should be the sum)
    check_contains_stack(function, 216, {20, 0});
    
    //Basic Block 8 (contains inlined functions)
    check_contains_inline_stack(function, 27, 11, "simple.c", "compute_sum", 1348);
    check_contains_inline_stack(function, 27, 12, "simple.c", "compute_sum", 2419);

    //Basic Block 11 (contains function inlined from standard library)
    check_contains_inline_stack(function, 29, 105, "stdio2.h", "printf", 0);
}

BOOST_AUTO_TEST_CASE( simple_c_lbr ){
    gooda::options options;
    parse_options(options, "--lbr", "tests/cases/simple-c/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple-c/lbr/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);

    auto& function = data.functions.front();

    //Verify function properties
    BOOST_CHECK_EQUAL(function.name, "main");
    BOOST_CHECK_EQUAL(function.file, "simple.c");
    BOOST_CHECK_EQUAL(function.total_count, 939346);
    BOOST_CHECK_EQUAL(function.entry_count, 0);

    //Basic Block 3
    check_contains_stack(function, 398, {24, 2});
    check_contains_stack(function, 0, {23, 0});
    check_contains_stack(function, 398, {23, 2});

    //Basic Block 7 (No discriminator here, should be the sum)
    check_contains_stack(function, 93775, {20, 0});
    
    //Basic Block 8 (contains inlined functions)
    check_contains_inline_stack(function, 27, 11, "simple.c", "compute_sum", 93776);
    check_contains_inline_stack(function, 27, 12, "simple.c", "compute_sum", 93776);

    //Basic Block 11 (contains function inlined from standard library)
    check_contains_inline_stack(function, 29, 105, "stdio2.h", "printf", 0);
}

BOOST_AUTO_TEST_CASE( inheritance_ucc ){
    gooda::options options;
    parse_options(options, "--nows", "tests/cases/inheritance/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/inheritance/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);

    auto& function = data.functions.front();

    //Verify function properties
    BOOST_CHECK_EQUAL(function.name, "main");
    BOOST_CHECK_EQUAL(function.file, "inheritance.cpp");
    BOOST_CHECK_EQUAL(function.total_count, 86404);
    BOOST_CHECK_EQUAL(function.entry_count, 0);

    //Basic Block 3
    check_contains_stack(function, 34, {29, 2});
    check_contains_stack(function, 0, {28, 0});
    check_contains_stack(function, 0, {28, 2});

    //TODO
}

BOOST_AUTO_TEST_SUITE_END()
