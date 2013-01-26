//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
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

    options.parse(6, argv);
    options.notify();
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

    std::reverse(positions.begin(), positions.end());

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
    parse_options(options, "--auto", "tests/cases/simple/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);
    BOOST_CHECK_EQUAL (report.processes(), 7);

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
    parse_options(options, "--auto", "tests/cases/simple/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple/lbr/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);
    BOOST_CHECK_EQUAL (report.processes(), 6);

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
    parse_options(options, "--auto", "tests/cases/simple-c/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple-c/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);
    BOOST_CHECK_EQUAL (report.processes(), 6);

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
    parse_options(options, "--auto", "tests/cases/simple-c/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/simple-c/lbr/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);
    BOOST_CHECK_EQUAL (report.processes(), 6);

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
    parse_options(options, "--auto", "tests/cases/inheritance/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/inheritance/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 1);
    BOOST_CHECK_EQUAL (report.processes(), 7);

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
    BOOST_CHECK_EQUAL (report.processes(), 6);

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
    parse_options(options, "--auto", "tests/cases/deep/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/deep/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 5);
    BOOST_CHECK_EQUAL (report.processes(), 7);

    {
        auto& function = data.functions[0];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z7computeILi4EEll");
        BOOST_CHECK_EQUAL(function.file, "deep_sum.hpp");
        BOOST_CHECK_EQUAL(function.total_count, 9512);
        BOOST_CHECK_EQUAL(function.entry_count, 59);

        check_contains_stack(function, 1054, {
                {"_Z7computeILi4EEll", "deep_sum.hpp", 6},
                {"compute<3>", "deep_sum.hpp", 4},
                {"compute<2>", "deep_sum.hpp", 6},
                {"compute_sum", "deep_sum.hpp", 2},
                {"compute_sum", "deep_compute.hpp", 17},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }

    {
        auto& function = data.functions[1];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z7computeILi1EEll");
        BOOST_CHECK_EQUAL(function.file, "deep_sum.hpp");
        BOOST_CHECK_EQUAL(function.total_count, 6959);
        BOOST_CHECK_EQUAL(function.entry_count, 273);

        check_contains_stack(function, 1420, {
                {"_Z7computeILi1EEll", "deep_sum.hpp", 6},
                {"compute_sum", "deep_sum.hpp", 2},
                {"compute_sum", "deep_compute.hpp", 17},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }

    {
        auto& function = data.functions[2];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z11compute_sumll");
        BOOST_CHECK_EQUAL(function.file, "deep_compute.hpp");
        BOOST_CHECK_EQUAL(function.total_count, 736);
        BOOST_CHECK_EQUAL(function.entry_count, 11);
        
        check_contains_stack(function, 128, {
                {"_Z11compute_sumll", "deep_compute.hpp", 13},
                {"compute_sum", "deep_compute.hpp", 17},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }

    {
        auto& function = data.functions[3];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z11compute_sumll.part.0");
        BOOST_CHECK_EQUAL(function.file, "deep_compute.hpp");
        BOOST_CHECK_EQUAL(function.total_count, 997);
        BOOST_CHECK_EQUAL(function.entry_count, 997);
        
        check_contains_stack(function, 469, {
                {"compute_sum", "deep_compute.hpp", 17},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }

    {
        auto& function = data.functions[4];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "main");
        BOOST_CHECK_EQUAL(function.file, "deep.cpp");
        BOOST_CHECK_EQUAL(function.total_count, 771);
        BOOST_CHECK_EQUAL(function.entry_count, 0);
        
        check_contains_stack(function, 0, {
                {"main", "deep.cpp", 9},
                {"compute<10>", "deep_sum.hpp", 6},
                {"compute<9>", "deep_sum.hpp", 6},
                {"compute<8>", "deep_sum.hpp", 6},
                {"compute<7>", "deep_sum.hpp", 6},
                {"compute<6>", "deep_sum.hpp", 6},
                {"compute<5>", "deep_sum.hpp", 6},
                {"compute_sum", "deep_compute.hpp", 15},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }
}

BOOST_AUTO_TEST_CASE( deep_lbr ){
    gooda::options options;
    parse_options(options, "--auto", "tests/cases/deep/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/deep/lbr/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 5);
    BOOST_CHECK_EQUAL (report.processes(), 6);

    {
        auto& function = data.functions[0];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z7computeILi4EEll");
        BOOST_CHECK_EQUAL(function.file, "deep_sum.hpp");
        BOOST_CHECK_EQUAL(function.total_count, 278935);
        BOOST_CHECK_EQUAL(function.entry_count, 1271);

        check_contains_stack(function, 557, {
                {"_Z7computeILi4EEll", "deep_sum.hpp", 6},
                {"compute<3>", "deep_sum.hpp", 4},
                {"compute<2>", "deep_sum.hpp", 6},
                {"compute_sum", "deep_sum.hpp", 2},
                {"compute_sum", "deep_compute.hpp", 17},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }

    {
        auto& function = data.functions[1];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z7computeILi1EEll");
        BOOST_CHECK_EQUAL(function.file, "deep_sum.hpp");
        BOOST_CHECK_EQUAL(function.total_count, 214674);
        BOOST_CHECK_EQUAL(function.entry_count, 4861);

        check_contains_stack(function, 2174, {
                {"_Z7computeILi1EEll", "deep_sum.hpp", 6},
                {"compute_sum", "deep_sum.hpp", 2},
                {"compute_sum", "deep_compute.hpp", 17},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }

    {
        auto& function = data.functions[2];
        
        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "main");
        BOOST_CHECK_EQUAL(function.file, "deep.cpp");
        BOOST_CHECK_EQUAL(function.total_count, 39251);
        BOOST_CHECK_EQUAL(function.entry_count, 0);
        
        check_contains_stack(function, 18, {
                {"main", "deep.cpp", 9},
                {"compute<10>", "deep_sum.hpp", 6},
                {"compute<9>", "deep_sum.hpp", 6},
                {"compute<8>", "deep_sum.hpp", 6},
                {"compute<7>", "deep_sum.hpp", 6},
                {"compute<6>", "deep_sum.hpp", 6},
                {"compute<5>", "deep_sum.hpp", 6},
                {"compute_sum", "deep_compute.hpp", 15},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }

    {
        auto& function = data.functions[3];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z11compute_sumll");
        BOOST_CHECK_EQUAL(function.file, "deep_compute.hpp");
        BOOST_CHECK_EQUAL(function.total_count, 22390);
        BOOST_CHECK_EQUAL(function.entry_count, 645);
        
        check_contains_stack(function, 340, {
                {"_Z11compute_sumll", "deep_compute.hpp", 13},
                {"compute_sum", "deep_compute.hpp", 17},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }

    {
        auto& function = data.functions[4];

        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "_Z11compute_sumll.part.0");
        BOOST_CHECK_EQUAL(function.file, "deep_compute.hpp");
        BOOST_CHECK_EQUAL(function.total_count, 23435);
        BOOST_CHECK_EQUAL(function.entry_count, 545);
        
        check_contains_stack(function, 545, {
                {"compute_sum", "deep_compute.hpp", 17},
                {"compute_third", "deep_compute.hpp", 10}
        });
    }
}

BOOST_AUTO_TEST_CASE( area_ucc ){
    gooda::options options;
    parse_options(options, "--auto", "tests/cases/area/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/area/ucc/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 2);
    BOOST_CHECK_EQUAL (report.processes(), 8);
    
    {
        auto& function = data.functions[0];
        
        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "__triangle_operations_MOD_area");
        BOOST_CHECK_EQUAL(function.file, "area.f90");
        BOOST_CHECK_EQUAL(function.total_count, 15654);
        BOOST_CHECK_EQUAL(function.entry_count, 14098);

        check_contains_stack(function, 143, {{"__triangle_operations_MOD_area", "area.f90", 4, 0}});
        check_contains_stack(function, 13945, {{"__triangle_operations_MOD_area", "area.f90", 8, 0}});
        check_contains_stack(function, 1301, {{"__triangle_operations_MOD_area", "area.f90", 9, 0}});
        check_contains_stack(function, 249, {{"__triangle_operations_MOD_area", "area.f90", 10, 0}});
        
    }

    {
        auto& function = data.functions[1];
        
        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "MAIN__");
        BOOST_CHECK_EQUAL(function.file, "area.f90");
        BOOST_CHECK_EQUAL(function.total_count, 2333);
        BOOST_CHECK_EQUAL(function.entry_count, 0);

        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 13, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 20, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 22, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 23, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 24, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 25, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 25, 0}});
        check_contains_stack(function, 453, {{"MAIN__", "area.f90", 26, 0}});
        check_contains_stack(function, 704, {{"MAIN__", "area.f90", 27, 0}});
        check_contains_stack(function, 1168, {{"MAIN__", "area.f90", 28, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 33, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 35, 0}});
    }
}

BOOST_AUTO_TEST_CASE( area_lbr ){
    gooda::options options;
    parse_options(options, "--auto", "tests/cases/area/");

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets("tests/cases/area/lbr/spreadsheets");

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, options.vm);

    BOOST_CHECK_EQUAL (data.functions.size(), 2);
    BOOST_CHECK_EQUAL (report.processes(), 7);
    
    {
        auto& function = data.functions[0];
        
        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "__triangle_operations_MOD_area");
        BOOST_CHECK_EQUAL(function.file, "area.f90");
        BOOST_CHECK_EQUAL(function.total_count, 811626);
        BOOST_CHECK_EQUAL(function.entry_count, 25138);

        check_contains_stack(function, 25138, {{"__triangle_operations_MOD_area", "area.f90", 4, 0}});
        check_contains_stack(function, 25138, {{"__triangle_operations_MOD_area", "area.f90", 8, 0}});
        check_contains_stack(function, 43467, {{"__triangle_operations_MOD_area", "area.f90", 9, 0}});
        check_contains_stack(function, 35075, {{"__triangle_operations_MOD_area", "area.f90", 10, 0}});
    }

    {
        auto& function = data.functions[1];
        
        //Verify function properties
        BOOST_CHECK_EQUAL(function.name, "MAIN__");
        BOOST_CHECK_EQUAL(function.file, "area.f90");
        BOOST_CHECK_EQUAL(function.total_count, 555732);
        BOOST_CHECK_EQUAL(function.entry_count, 0);

        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 13, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 20, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 22, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 23, 0}});
        check_contains_stack(function, 32, {{"MAIN__", "area.f90", 24, 0}});
        check_contains_stack(function, 28, {{"MAIN__", "area.f90", 25, 0}});
        check_contains_stack(function, 28, {{"MAIN__", "area.f90", 25, 0}});
        check_contains_stack(function, 37310, {{"MAIN__", "area.f90", 26, 0}});
        check_contains_stack(function, 37310, {{"MAIN__", "area.f90", 27, 0}});
        check_contains_stack(function, 37310, {{"MAIN__", "area.f90", 28, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 33, 0}});
        check_contains_stack(function, 0, {{"MAIN__", "area.f90", 35, 0}});
    }
}


BOOST_AUTO_TEST_SUITE_END()
