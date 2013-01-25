//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "diff.hpp"
#include "logger.hpp"
#include "hash.hpp"

void gooda::diff(const gooda_report& first, const gooda_report& second, boost::program_options::variables_map&){
    auto& first_file = first.get_hotspot_file();
    auto& second_file = second.get_hotspot_file();

    for(std::size_t i = 0; i < first.functions(); ++i){
        auto& first_line = first.hotspot_function(i);
        auto first_name = first_line.get_string(first_file.column(FUNCTION_NAME));

        bool found = false;

        for(std::size_t j = 0; j < second.functions(); ++j){
            auto& second_line = second.hotspot_function(j);
            auto second_name = second_line.get_string(second_file.column(FUNCTION_NAME));

            if(first_name == second_name){
                auto diff = first_line.get_counter(first_file.column(UNHALTED_CORE_CYCLES)) - second_line.get_counter(second_file.column(UNHALTED_CORE_CYCLES));

                std::cout << "Diff " << first_name << ": " << diff << " unhalted core cycles" << std::endl;

                found = true;
                break;
            }
        }

        if(!found){
            std::cout << "Diff " << first_name << ": 0 unhalted core cycles" << std::endl;
        }
    }
    
    for(std::size_t i = 0; i < second.functions(); ++i){
        auto& second_line = second.hotspot_function(i);
        auto second_name = second_line.get_string(second_file.column(FUNCTION_NAME));
        
        bool found = false;

        for(std::size_t j = 0; j < first.functions(); ++j){
            auto& first_line = first.hotspot_function(j);
            auto first_name = first_line.get_string(first_file.column(FUNCTION_NAME));

            if(first_name == second_name){
                found = true;
                break;
            }
        }


        if(!found){
            std::cout << "Diff " << second_name << ": 0 unhalted core cycles" << std::endl;
        }
    }
}
