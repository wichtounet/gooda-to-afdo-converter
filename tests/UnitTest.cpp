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

struct P {
    std::string func;
    std::string file;
    unsigned int line;
    unsigned int discr;

    P(std::string func, std::string file, unsigned int line, unsigned int discr = 0) : func(func), file(file), line(line), discr(discr) {}
};

void check_contains_stack(const gooda::afdo_function& function, std::size_t count, std::vector<P> positions){
    bool found = false;

    for(auto& stack : function.stacks){
        if(stack.stack.size() == positions.size()){
            bool exact = true;

            for(std::size_t i = 0; i < stack.stack.size(); ++i){
                auto& pos1 = stack.stack[i];
                auto& pos = positions[i];

                if(pos1.line != pos.line || pos1.discriminator != pos.discr || pos1.file != pos.file || pos1.func != pos.func){
                    exact = false;
                    break;
                }
            }

            if(exact){
                BOOST_CHECK(!found);

                BOOST_CHECK_EQUAL(stack.count, count);

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
    check_contains_stack(function, 11, {{"main", "simple.cpp", 22, 2}});

    //Basic Block 7 (No discriminator here, should be the sum)
    check_contains_stack(function, 329, {{"main", "simple.cpp", 20, 0}});

    //Basic Block 8 (contains inlined functions)
    check_contains_stack(function, 787, {{"main", "simple.cpp", 26}, {"compute_sum", "simple.cpp", 10}});
    check_contains_stack(function, 2817,{{"main", "simple.cpp", 26}, {"compute_sum", "simple.cpp", 11}});

    //Basic Block 11 (contains function inlined from standard library)
    check_contains_stack(function, 0, {{"main", "simple.cpp", 28}, {"_ZNSolsEPFRSoS_E", "ostream", 111}});
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
    check_contains_stack(function, 360, {{"main", "simple.cpp", 22, 2}});
    check_contains_stack(function, 360, {{"main", "simple.cpp", 23, 2}});
    
    //Basic Block 4 (Verify that the lines with different discriminators are different)
    check_contains_stack(function, 0, {{"main", "simple.cpp", 22, 0}});

    //Basic Block 7
    check_contains_stack(function, 93795, {{"main", "simple.cpp", 20, 0}});

    //Basic Block 8 (contains inlined functions)
    check_contains_stack(function, 93795, {{"main", "simple.cpp", 26, 0}, {"compute_sum", "simple.cpp", 10, 0}});
    check_contains_stack(function, 93795, {{"main", "simple.cpp", 26, 0}, {"compute_sum", "simple.cpp", 11, 0}});

    //Basic Block 11 (contains function inlined from standard library)
    check_contains_stack(function, 0, {{"main", "simple.cpp", 28, 0}, {"_ZNSolsEPFRSoS_E", "ostream", 111, 0}});
    check_contains_stack(function, 0, {{"main", "simple.cpp", 28, 0}, {"_ZNSolsEl", "ostream", 165, 0}});
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
    check_contains_stack(function, 91, {{"main", "simple.c", 24, 2}});
    check_contains_stack(function, 0, {{"main", "simple.c", 23, 0}});
    check_contains_stack(function, 0, {{"main", "simple.c", 23, 2}});

    //Basic Block 7 (No discriminator here, should be the sum)
    check_contains_stack(function, 216, {{"main", "simple.c", 20, 0}});
    
    //Basic Block 8 (contains inlined functions)
    check_contains_stack(function, 1348, {{"main", "simple.c", 27, 0}, {"compute_sum", "simple.c", 11, 0}});
    check_contains_stack(function, 2419, {{"main", "simple.c", 27, 0}, {"compute_sum", "simple.c", 12, 0}});

    //Basic Block 11 (contains function inlined from standard library)
    check_contains_stack(function, 0, {{"main", "simple.c", 29, 0}, {"printf", "stdio2.h", 105, 0}});
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
    check_contains_stack(function, 398, {{"main", "simple.c", 24, 2}});
    check_contains_stack(function, 0, {{"main", "simple.c", 23, 0}});
    check_contains_stack(function, 398, {{"main", "simple.c", 23, 2}});

    //Basic Block 7 (No discriminator here, should be the sum)
    check_contains_stack(function, 93775, {{"main", "simple.c", 20, 0}});
    
    //Basic Block 8 (contains inlined functions)
    check_contains_stack(function, 93776, {{"main", "simple.c", 27, 0}, {"compute_sum", "simple.c", 11, 0}});
    check_contains_stack(function, 93776, {{"main", "simple.c", 27, 0}, {"compute_sum", "simple.c", 12, 0}});

    //Basic Block 11 (contains function inlined from standard library)
    check_contains_stack(function, 0, {{"main", "simple.c", 29, 0}, {"printf", "stdio2.h", 105, 0}});
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
    check_contains_stack(function, 34, {{"main", "inheritance.cpp", 29, 2}});
    check_contains_stack(function, 0, {{"main", "inheritance.cpp", 28, 0}});
    check_contains_stack(function, 0, {{"main", "inheritance.cpp", 28, 2}});
    
    //Basic Block 8
    check_contains_stack(function, 0, {{"main", "inheritance.cpp", 33, 0}, {"_ZN1A11compute_sumEiPl", "inheritance.cpp", 13, 0}});
    check_contains_stack(function, 0, {{"main", "inheritance.cpp", 33, 0}});
    check_contains_stack(function, 72582, {{"main", "inheritance.cpp", 33, 0}, {"_ZN1A11compute_sumEiPl", "inheritance.cpp", 11, 0}});
    check_contains_stack(function, 2394, {{"main", "inheritance.cpp", 33}, {"_ZN1A11compute_sumEiPl", "inheritance.cpp", 10}});
    check_contains_stack(function, 11383, {{"main", "inheritance.cpp", 33}, {"_ZN1A11compute_sumEiPl", "inheritance.cpp", 11}, {"_ZN1A11compute_sumEiil", "inheritance.cpp", 18}});
}

BOOST_AUTO_TEST_CASE( inheritance_lbr ){
    gooda::options options;
    parse_options(options, "--lbr", "tests/cases/inheritance/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/inheritance/lbr/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);

    auto& function = data.functions.front();

    //Verify function properties
    BOOST_CHECK_EQUAL(function.name, "main");
    BOOST_CHECK_EQUAL(function.file, "inheritance.cpp");
    BOOST_CHECK_EQUAL(function.total_count, 3565925);
    BOOST_CHECK_EQUAL(function.entry_count, 0);

    //Basic Block 3
    check_contains_stack(function, 360, {{"main", "inheritance.cpp", 29, 2}});
    check_contains_stack(function, 0, {{"main", "inheritance.cpp", 28, 0}});
    check_contains_stack(function, 360, {{"main", "inheritance.cpp", 28, 2}});
    
    //Basic Block 8
    check_contains_stack(function, 0, {{"main", "inheritance.cpp", 33, 0}, {"_ZN1A11compute_sumEiPl", "inheritance.cpp", 13, 0}});
    check_contains_stack(function, 0, {{"main", "inheritance.cpp", 33, 0}});
    check_contains_stack(function, 187608, {{"main", "inheritance.cpp", 33, 0}, {"_ZN1A11compute_sumEiPl", "inheritance.cpp", 11, 0}});
    check_contains_stack(function, 187608, {{"main", "inheritance.cpp", 33}, {"_ZN1A11compute_sumEiPl", "inheritance.cpp", 10}});
    check_contains_stack(function, 187608, {{"main", "inheritance.cpp", 33}, {"_ZN1A11compute_sumEiPl", "inheritance.cpp", 11}, {"_ZN1A11compute_sumEiil", "inheritance.cpp", 18}});
}

BOOST_AUTO_TEST_CASE( deep_ucc ){
    gooda::options options;
    parse_options(options, "--nows", "tests/cases/deep/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/deep/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 5);

    {
        auto& function = data.functions[0];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z7computeILi4EEll");
        BOOST_CHECK_EQUAL(function.file, "deep.cpp");
        BOOST_CHECK_EQUAL(function.total_count, 9375);
        BOOST_CHECK_EQUAL(function.entry_count, 47);
    }

    {
        auto& function = data.functions[1];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "compute<0>");
        BOOST_CHECK_EQUAL(function.file, "deep.cpp");
        BOOST_CHECK_EQUAL(function.total_count, 6752);
        BOOST_CHECK_EQUAL(function.entry_count, 308);
    }

    {
        auto& function = data.functions[2];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "compute_third");
        BOOST_CHECK_EQUAL(function.file, "deep.cpp");
        BOOST_CHECK_EQUAL(function.total_count, 949);
        BOOST_CHECK_EQUAL(function.entry_count, 949);
    }

    {
        auto& function = data.functions[3];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z11compute_sumll");
        BOOST_CHECK_EQUAL(function.file, "deep.cpp");
        BOOST_CHECK_EQUAL(function.total_count, 1044);
        BOOST_CHECK_EQUAL(function.entry_count, 0);
    }

    {
        auto& function = data.functions[4];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "main");
        BOOST_CHECK_EQUAL(function.file, "deep.cpp");
        BOOST_CHECK_EQUAL(function.total_count, 854);
        BOOST_CHECK_EQUAL(function.entry_count, 0);
    }
}

BOOST_AUTO_TEST_SUITE_END()
