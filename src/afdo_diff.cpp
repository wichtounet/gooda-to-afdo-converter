//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file afdo_diff.cpp
 * \brief Implementation of a diff between two AFDO profiles. 
 */

#include "afdo_diff.hpp"

void diff(gooda::afdo_function& first, gooda::afdo_function& second){
    //TODO
}

void gooda::afdo_diff(const afdo_data& first, const afdo_data& second, boost::program_options::variables_map&){
    if(first.functions.size() > second.functions.size()){
        std::cout << "First has " << (first.functions.size() - second.functions.size()) << " more hotpot functions than second" << std::endl;
    } else if(second.functions.size() > first.functions.size()){
        std::cout << "Second has " << (second.functions.size() - first.functions.size()) << " more hotpot functions than first" << std::endl;
    } else {
        std::cout << "Both profiles have the same number of hotspot functions" << std::endl;
    }

    auto limit = std::min(20UL, std::min(first.functions.size(), second.functions.size()));

    std::cout << "Performing diff on the " << limit << " hottest functions" << std::endl;

    auto first_functions = first.functions;
    auto second_functions = second.functions;
    
    std::sort(first_functions.begin(), first_functions.end(), [](const gooda::afdo_function& lhs, const gooda::afdo_function& rhs){ return lhs.total_count > rhs.total_count; });
    std::sort(second_functions.begin(), second_functions.end(), [](const gooda::afdo_function& lhs, const gooda::afdo_function& rhs){ return lhs.total_count > rhs.total_count; });

    for(std::size_t i = 0; i < limit; ++i){
        if(first_functions[i].name != second_functions[i].name){
            std::cout << "   " << i << "th hottest function is not the same" << std::endl;
            std::cout << "     First : " << first_functions[i].name << std::endl;
            std::cout << "     Second : " << second_functions[i].name << std::endl;

            bool found = false;

            for(std::size_t j = 0; j < second_functions.size(); ++j){
                if(second_functions[j].name == first_functions[i].name){
                    std::cout << "     " << first_functions[i].name << " was found in Second, " << j << "th hottest" << std::endl;
                    
                    auto& first_function = first_functions[i];
                    auto& second_function = second_functions[j];

                    diff(first_function, second_function);

                    found = true;
                    break;
                }
            }
                    
            if(!found){
                std::cout << "     " << first_functions[i].name << " is not present in Second" << std::endl;
            }

            found = false;
            
            for(std::size_t j = 0; j < first_functions.size(); ++j){
                if(second_functions[i].name == first_functions[j].name){
                    std::cout << "     " << first_functions[j].name << " was found in Second, " << j << "th hottest" << std::endl;
                    
                    auto& first_function = first_functions[j];
                    auto& second_function = second_functions[i];

                    diff(first_function, second_function);

                    found = true;
                    break;
                }
            }
                    
            if(!found){
                std::cout << "     " << second_functions[i].name << " is not present in First" << std::endl;
            }
        } else {
            auto& first_function = first_functions[i];
            auto& second_function = second_functions[i];

            diff(first_function, second_function);
        }
    }
}
