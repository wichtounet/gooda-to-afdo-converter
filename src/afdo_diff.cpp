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
#include "assert.hpp"

bool same_pos(gooda::afdo_pos& lhs, gooda::afdo_pos& rhs){
    if(lhs.line != rhs.line || lhs.discriminator != rhs.discriminator || lhs.file != rhs.file){
        return false;   
    }

    if(lhs.func == rhs.func){
        return true;
    }

    auto shortest = lhs.func.size() > rhs.func.size() ? rhs.func : lhs.func;
    auto longest = lhs.func.size() > rhs.func.size() ? lhs.func : rhs.func;

    return longest.find(shortest) != std::string::npos;
}

bool same_stack(gooda::afdo_stack& lhs, gooda::afdo_stack& rhs){
    if(lhs.stack.size() == rhs.stack.size()){
        for(std::size_t i = 0; i < rhs.stack.size(); ++i){
            if(!same_pos(lhs.stack[i], rhs.stack[i])){
                return false;
            }
        }

        return true;
    }

    return false;
}

bool has_stack(gooda::afdo_function& function, gooda::afdo_stack& stack){
    for(auto& s : function.stacks){
        if(same_stack(s, stack)){
            return true;
        }
    }

    return false;
}

gooda::afdo_stack& get_stack(gooda::afdo_function& function, gooda::afdo_stack& stack){
    for(auto& s : function.stacks){
        if(same_stack(s, stack)){
            return s;
        }
    }

    gooda_unreachable("Should not be called when the stack is not present in the function");
}

void diff(gooda::afdo_function& first, gooda::afdo_function& second){
    std::cout << "Diff of " << first.name << " function " << std::endl;

    if(first.file != second.file){
        std::cout << "  File are the not same" << std::endl;
        std::cout << "    First: " << first.file << std::endl;
        std::cout << "    Second: " << second.file << std::endl;
    }

    if(first.total_count != second.total_count){
        double min = std::min(first.total_count, second.total_count);
        double max = std::max(first.total_count, second.total_count);
        
        std::cout << "  total_count differs (" << first.total_count << " and " << second.total_count << ")" << std::endl;
        std::cout << "    difference: " << (100 - 100 * (min / max)) << "% (" << (max - min) << ")" << std::endl;
    }
    
    if(first.entry_count != second.entry_count){
        double min = std::min(first.entry_count, second.entry_count);
        double max = std::max(first.entry_count, second.entry_count);
        
        std::cout << "  entry_count differs (" << first.entry_count << " and " << second.entry_count << ")" << std::endl;
        std::cout << "    difference: " << (100 - 100 * (min / max)) << "% (" << (max - min) << ")" << std::endl;
    }

    if(first.stacks.size() > second.stacks.size()){
        std::cout << "  First has " << (first.stacks.size() - second.stacks.size()) << " more inline stacks than second" << std::endl;
    } else if(second.stacks.size() > first.stacks.size()){
        std::cout << "  Second has " << (second.stacks.size() - first.stacks.size()) << " more inline stacks than first" << std::endl;
    }

    int not_in_second = 0;
    int not_in_first = 0;

    for(auto& first_stack : first.stacks){
        if(has_stack(second, first_stack)){
            auto& second_stack = get_stack(second, first_stack);

            if(first_stack.count != second_stack.count){
                double min = std::min(first_stack.count, second_stack.count);
                double max = std::max(first_stack.count, second_stack.count);

                std::cout << "  count differs (" << first_stack.count << " and " << second_stack.count << ")" << std::endl;
                std::cout << "    difference: " << (100 - 100 * (min / max)) << "% (" << (max - min) << ")" << std::endl;
            }
        } else {
            ++not_in_second;
        }
    }

    for(auto& second_stack : second.stacks){
        if(!has_stack(first, second_stack)){
            ++not_in_first;
        }
    }
    
    std::cout << "  " << not_in_second << " inline stack present in first are not in second" << std::endl;
    std::cout << "  " << not_in_first << " inline stack present in second are not in first" << std::endl;


    std::cout << std::endl;
}

void gooda::afdo_diff(const afdo_data& first, const afdo_data& second, boost::program_options::variables_map&){
    if(first.functions.size() > second.functions.size()){
        std::cout << "First has " << (first.functions.size() - second.functions.size()) << " more hotpot functions than second" << std::endl<< std::endl;
    } else if(second.functions.size() > first.functions.size()){
        std::cout << "Second has " << (second.functions.size() - first.functions.size()) << " more hotpot functions than first" << std::endl<< std::endl;
    } else {
        std::cout << "Both profiles have the same number of hotspot functions" << std::endl << std::endl;
    }

    auto limit = std::min(5UL, std::min(first.functions.size(), second.functions.size()));

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
