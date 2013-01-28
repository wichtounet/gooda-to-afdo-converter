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

#include <iostream>

#include "afdo_diff.hpp"
#include "assert.hpp"
#include "afdo_printer.hpp"

/*!
 * \brief Test if two afdo_pos are referring to the same positions. Support mix of bfd and assembler names. 
 * \param lhs The first object to compare
 * \param rhs The second object to compare
 * \return true if both afdo_pos are referring to the same position, false otherwise.
 */
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

/*!
 * \brief Test if two afdo_stack are referring to the same positions. Support mix of bfd and assembler names. 
 * \param lhs The first object to compare
 * \param rhs The second object to compare
 * \return true if both afdo_stack are referring to the same position, false otherwise. 
 */
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

void print_difference(std::string name, long first, long second){
    if(first != second){
        std::cout << "   " << name << " differs (" << first << " and " << second << ")" << std::endl;
        
        double difference = 100 * (static_cast<double>(first) / static_cast<double>(second)) - 100;
        if(difference > 0){
            std::cout << "    difference: +" << (difference) << "% (+" << (first - second) << ")" << std::endl;
        } else {
            std::cout << "    difference: " << (difference) << "% (" << (first - second) << ")" << std::endl;
        }
    }
}

void diff(const gooda::afdo_data& first_data, const gooda::afdo_data& second_data, gooda::afdo_function& first, gooda::afdo_function& second, boost::program_options::variables_map& vm){
    std::cout << "Diff of " << first.name << " function " << std::endl;

    if(first.file != second.file){
        std::cout << "  File are the not same" << std::endl;
        std::cout << "    First: " << first.file << std::endl;
        std::cout << "    Second: " << second.file << std::endl;
    }

    print_difference("total_count", first.total_count, second.total_count);
    print_difference("entry_count", first.entry_count, second.entry_count);

    int not_in_second = 0;
    int not_in_first = 0;

    for(auto& first_stack : first.stacks){
        if(has_stack(second, first_stack)){
            auto& second_stack = get_stack(second, first_stack);
            
            print_difference("count", first_stack.count, second_stack.count);
        } else {
            ++not_in_second;
        }
    }

    for(auto& second_stack : second.stacks){
        if(!has_stack(first, second_stack)){
            ++not_in_first;
        }
    }

    if(first.stacks.size() > second.stacks.size()){
        std::cout << "  First has " << (first.stacks.size() - second.stacks.size()) << " more inline stacks than second" << std::endl;
    } else if(second.stacks.size() > first.stacks.size()){
        std::cout << "  Second has " << (second.stacks.size() - first.stacks.size()) << " more inline stacks than first" << std::endl;
    }
    
    if(not_in_second > 0){
        std::cout << "  " << not_in_second << " inline stack present in first are not in second" << std::endl;
        for(auto& first_stack : first.stacks){
            if(!has_stack(second, first_stack)){
                dump_afdo(first_data, first_stack, vm);
            }
        }
    }

    if(not_in_first > 0 ){
        std::cout << "  " << not_in_first << " inline stack present in second are not in first" << std::endl;
        for(auto& second_stack : second.stacks){
            if(!has_stack(first, second_stack)){
                dump_afdo(second_data, second_stack, vm);
            }
        }
    }

    std::cout << std::endl;
}

void gooda::afdo_diff(const afdo_data& first, const afdo_data& second, boost::program_options::variables_map& vm){
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

                    diff(first, second, first_function, second_function, vm);

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

                    diff(first, second, first_function, second_function, vm);

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

            diff(first, second, first_function, second_function, vm);
        }
    }
}
