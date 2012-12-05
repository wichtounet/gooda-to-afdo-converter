//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <map>

#include <boost/algorithm/string.hpp>

#include "assert.hpp"
#include "converter.hpp"
#include "utils.hpp"
#include "logger.hpp"

template <class T>
inline void hash_combine(std::size_t& seed, const T& v){
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

typedef std::pair<std::string, std::size_t> inlined_key;

namespace std {

template<>
class hash<inlined_key> {
    public:
        std::size_t operator()(const inlined_key& key) const {
            std::size_t seed = 0;

            hash_combine(seed, key.first);
            hash_combine(seed, key.second);

            return seed;
        }
};

} //end of namespace std

namespace {

//Common utilities

struct gooda_bb {
    std::string file;
    unsigned long line_start;
    unsigned long exec_count;
    long address;

    std::size_t gooda_function;
    std::size_t gooda_line_start;   //Inside asm_file
    std::size_t gooda_line_end;     //Inside asm_file
    
    //If the basic block comes from an inlined function
    std::string inlined_file;
    unsigned long inlined_line_start;
};

gooda::afdo_stack& get_inlined_stack(gooda::afdo_function& function, std::string src_func, std::string src_file, std::size_t src_line, std::string dest_func, std::string dest_file, std::size_t dest_line){
    for(auto& stack : function.stacks){
        if(stack.stack.size() == 2){
            auto& src_pos = stack.stack.front();

            if(src_pos.func == src_func && src_pos.file == src_file && src_pos.line == src_line){
                auto& dest_pos = stack.stack.back();
            
                if(dest_pos.func == dest_func && dest_pos.file == dest_file && dest_pos.line == dest_line){
                    return stack;
                }
            }
        }
    }

    gooda::afdo_pos src_position;
    src_position.func = src_func;
    src_position.file = src_file;
    src_position.line = src_line;
    src_position.discr = 0;

    gooda::afdo_pos dest_position;
    dest_position.func = dest_func;
    dest_position.file = dest_file;
    dest_position.line = dest_line;
    dest_position.discr = 0;
    
    gooda::afdo_stack stack;
    stack.count = 0;
    stack.num_inst = 0;

    stack.stack.push_back(std::move(src_position));
    stack.stack.push_back(std::move(dest_position));

    function.stacks.push_back(std::move(stack));

    return function.stacks.back(); 
}

gooda::afdo_stack& get_stack(gooda::afdo_function& function, std::string func, std::string file, std::size_t line){
    for(auto& stack : function.stacks){
        if(stack.stack.size() == 1){
            auto& pos = stack.stack.front();

            if(pos.func == func && pos.file == file && pos.line == line){
                return stack;
            }
        }
    }

    gooda::afdo_pos position;
    position.func = func;
    position.file = file;
    position.line = line;
    position.discr = 0;
    
    gooda::afdo_stack stack;
    stack.count = 0;
    stack.num_inst = 0;

    stack.stack.push_back(std::move(position));

    function.stacks.push_back(std::move(stack));

    return function.stacks.back(); 
}

typedef std::vector<gooda_bb> bb_vector;

std::vector<bb_vector> compute_inlined_sets(bb_vector block_set){
    std::unordered_map<inlined_key, bb_vector> inline_mappings;

    for(auto& block : block_set){
        //If this block comes from an inlined function
        if(!block.inlined_file.empty()){
            inline_mappings[{block.file, block.line_start}].push_back(std::move(block));
        }
    }
    
    std::vector<bb_vector> inlined_sets;

    for(auto& pair : inline_mappings){
       inlined_sets.push_back(std::move(pair.second)); 
    }

    return inlined_sets;
}

std::pair<bb_vector, std::vector<bb_vector>> split_bbs(bb_vector& basic_blocks){
    bb_vector normal_blocks;

    //Extract the normal blocks
    for(auto& block : basic_blocks){
        if(block.inlined_file.empty()){
            normal_blocks.push_back(std::move(block));
        }
    }

    auto inlined_block_sets = compute_inlined_sets(basic_blocks);

    //Note: From this point basic_blocks contains invalid basic_blocks due to std::moving them
    basic_blocks.clear();
    
    return std::make_pair(normal_blocks, inlined_block_sets);
}

std::string get_function_name(const std::string& application_file, bb_vector& block_set){
    log::emit<log::Debug>() << "Get function name with objdump" << log::endl;

    long start_address = std::numeric_limits<long>::max();
    for(auto& block : block_set){
        start_address = std::min(start_address, block.address);
    }

    long stop_address = start_address + 3;
    
    std::stringstream ss;
    ss << "objdump " << application_file << " -D --line-numbers --start-address=0x" << std::hex << start_address << " --stop-address=0x" << stop_address;
    
    log::emit<log::Debug>() << "=> Command:" << ss.str() << log::endl;

    std::string command = ss.str();
    auto result = gooda::exec_command_result(command);

    std::istringstream result_stream(result);
    std::string str_line;    
    bool next = false;
    std::string function_name;

    while (std::getline(result_stream, str_line)) {
        if(boost::starts_with(str_line, "00000")){
            next = true;
        } else if(next){
            function_name = str_line; 
            break;
        }
    }

    function_name = function_name.substr(0, function_name.size() - 3);

    log::emit<log::Debug>() << "Found \"" << function_name << "\"" << log::endl;

    return function_name;
}

bb_vector collect_basic_blocks(const gooda::gooda_report& report, gooda::afdo_data& data, gooda::afdo_function& function, const std::string& counter_name){
    bb_vector basic_blocks;

    if(report.has_asm_file(function.i)){
        auto& file = report.asm_file(function.i);

        //Compute the addresses of the first and the last instructions
        auto start_instruction = report.hotspot_function(function.i).get_address(report.get_hotspot_file().column(OFFSET));
        auto length = report.hotspot_function(function.i).get_address(report.get_hotspot_file().column(LENGTH));
        auto last_instruction = start_instruction + length;

        bool bb_found = false;
        bool collection = false;

        function.first_line = std::numeric_limits<decltype(function.first_line)>::max();
        function.last_line = std::numeric_limits<decltype(function.last_line)>::min();

        for(std::size_t j = 0; j < file.lines(); ++j){
            auto& line = file.line(j);

            //Gooda does not always found the source file of a function
            //In that case, declare the function as invalid and return quickly
            if(line.get_string(file.column(PRINC_FILE)) == "null"){
                function.valid = false;
                log::emit<log::Warning>() << function.name << " is invalid (null file)" << log::endl;
                return {};
            }

            //It indicates the last line, that is not a valid assembly line but a summary of the data
            if(line.get_string(file.column(ADDRESS)).empty()){
                return basic_blocks;
            }

            auto disassembly = line.get_string(file.column(DISASSEMBLY));

            //Collect Basic Block informations
            if(boost::starts_with(disassembly, "Basic Block ")){
                gooda_bb block;

                block.file = line.get_string(file.column(PRINC_FILE));
                block.line_start = line.get_counter(file.column(PRINC_LINE));
                block.exec_count = line.get_counter(file.column(counter_name));
                block.address = line.get_address(file.column(ADDRESS));
                block.gooda_function = function.i;

                //Compute the start and end line
                block.gooda_line_start = j;

                auto k = j+1;
                while(k < file.lines() - 1 && !boost::starts_with(file.line(k).get_string(file.column(DISASSEMBLY)), "Basic Block")){
                    k++;
                }

                block.gooda_line_end = k == file.lines() ? k - 1 : k;

                //By default considered as not coming from inlined function
                block.inlined_line_start = 0;

                //Look at the next line to find out if the line comes from an inlined function
                if(j + 1 < file.lines()){
                    auto& next_line = file.line(j + 1);

                    //If the next line is part of the same basic block
                    if(next_line.get_counter(file.column(PRINC_LINE)) == block.line_start){
                        auto init_file = next_line.get_string(file.column(INIT_FILE));

                        if(!init_file.empty()){
                            block.inlined_line_start = next_line.get_counter(file.column(INIT_LINE));
                            block.inlined_file = init_file;
                        }
                    }
                }

                basic_blocks.push_back(std::move(block));
            } 
            
            //Get the entry basic block
            if(boost::starts_with(disassembly, "Basic Block 1 <")){
                function.entry_count = line.get_counter(file.column(counter_name));

                bb_found = true;
            } else if(bb_found){
                auto file_name = line.get_string(file.column(PRINC_FILE));

                function.file = file_name;
                data.add_file_name(file_name);

                bb_found = false;
            }

            auto address = line.get_address(file.column(ADDRESS)); 

            if(address == start_instruction){
                collection = true;
            } else if(address >= last_instruction){
                break;
            }
            
            //If we are inside the function
            if(collection){
                function.first_line = std::min(line.get_counter(file.column(PRINC_LINE)), function.first_line);
                function.last_line = std::max(line.get_counter(file.column(PRINC_LINE)), function.last_line);
            }
        }

        gooda_assert(!function.file.empty(), "The function file must be set");
        gooda_assert(function.first_line < std::numeric_limits<decltype(function.first_line)>::max(), "The function first line must be set");
        gooda_assert(function.last_line > std::numeric_limits<decltype(function.last_line)>::min(), "The function last line must be set");
    }

    return basic_blocks;
}

//Cycle Accounting mode

void ca_annotate(const gooda::gooda_report& report, gooda::afdo_data& data, gooda::afdo_function& function, bb_vector& basic_blocks){
    if(report.has_asm_file(function.i)){
        auto& asm_file = report.asm_file(function.i);

        bb_vector normal_blocks;
        std::vector<bb_vector> inlined_block_sets;
        std::tie(normal_blocks, inlined_block_sets) = split_bbs(basic_blocks);
        
        //1. Normal pass for non-inlined blocks
        for(auto& block : normal_blocks){
            for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
                gooda_assert(j < asm_file.lines(), "Something went wrong with BB collection");

                auto& asm_line = asm_file.line(j);
                auto line_number = asm_line.get_counter(asm_file.column(PRINC_LINE));

                auto& stack = get_stack(function, function.name, function.file, line_number);
                stack.count = std::max(stack.count, asm_line.get_counter(asm_file.column(UNHALTED_CORE_CYCLES)));
                ++stack.num_inst;
            }
        }
        
        //2. Handle inlined blocks if any
        for(auto& block_set : inlined_block_sets){
            gooda_assert(block_set.size() > 0, "Something went wrong with BB Collection");

            auto callee_function_name = get_function_name(data.application_file, block_set);

            for(auto& block : block_set){
                for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
                    gooda_assert(j < asm_file.lines(), "Something went wrong with BB collection");

                    auto& asm_line = asm_file.line(j);

                    //It is possible that a basic block is not made only 
                    //of inlined lines
                    if(asm_line.get_string(asm_file.column(INIT_FILE)).empty()){
                        auto& stack = get_stack(function, function.name, function.file, asm_line.get_counter(asm_file.column(PRINC_LINE))); 

                        stack.count = std::max(stack.count, asm_line.get_counter(asm_file.column(UNHALTED_CORE_CYCLES)));
                        ++stack.num_inst;
                    } else {
                        auto callee_line_number = asm_line.get_counter(asm_file.column(INIT_LINE));

                        auto& stack = get_inlined_stack(
                                function, 
                                function.name, function.file, asm_line.get_counter(asm_file.column(PRINC_LINE)),
                                callee_function_name, block.inlined_file, callee_line_number); 

                        data.add_file_name(callee_function_name);
                        data.add_file_name(block.inlined_file);

                        stack.count = std::max(stack.count, asm_line.get_counter(asm_file.column(UNHALTED_CORE_CYCLES)));
                        ++stack.num_inst;
                    }
                }
            }
        }
    }
}

//LBR Mode

void lbr_annotate(const gooda::gooda_report& report, gooda::afdo_data& data, gooda::afdo_function& function, bb_vector& basic_blocks){
    if(report.has_asm_file(function.i)){
        auto& asm_file = report.asm_file(function.i);

        bb_vector normal_blocks;
        std::vector<bb_vector> inlined_block_sets;
        std::tie(normal_blocks, inlined_block_sets) = split_bbs(basic_blocks);

        //1. Normal pass for non-inlined blocks
        for(auto& block : normal_blocks){
            for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
                gooda_assert(j < asm_file.lines(), "Something went wrong with BB collection");

                auto& asm_line = asm_file.line(j);
                auto line_number = asm_line.get_counter(asm_file.column(PRINC_LINE));

                auto& stack = get_stack(function, function.name, function.file, line_number);
                stack.count = std::max(stack.count, block.exec_count);
                ++stack.num_inst;
            }
        }

        //2. Handle inlined blocks if any
        if(!inlined_block_sets.empty()){
            for(auto& block_set : inlined_block_sets){
                gooda_assert(block_set.size() > 0, "Something went wrong with BB Collection");

                //TODO Perhaps it is faster to look out if it exists first
                auto callee_function_name = get_function_name(data.application_file, block_set);

                for(auto& block : block_set){
                    for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
                        gooda_assert(j < asm_file.lines(), "Something went wrong with BB collection");

                        auto& asm_line = asm_file.line(j);

                        //It is possible that a basic block is not made only 
                        //of inlined lines
                        if(asm_line.get_string(asm_file.column(INIT_FILE)).empty()){
                            auto& stack = get_stack(function, function.name, function.file, asm_line.get_counter(asm_file.column(PRINC_LINE))); 

                            stack.count = std::max(stack.count, block.exec_count);
                            ++stack.num_inst;
                        } else {
                            auto callee_line_number = asm_line.get_counter(asm_file.column(INIT_LINE));

                            auto& stack = get_inlined_stack(
                                    function, 
                                    function.name, function.file, asm_line.get_counter(asm_file.column(PRINC_LINE)),
                                    callee_function_name, block.inlined_file, callee_line_number); 

                            data.add_file_name(callee_function_name);
                            data.add_file_name(block.inlined_file);

                            stack.count = std::max(stack.count, block.exec_count);
                            ++stack.num_inst;
                        }
                    }
                }
            }
        }
    }
}

//Common functions 

unsigned int sizeof_string(const std::string& str){
    return 1 + (str.length() + sizeof(gcov_unsigned_t)) / sizeof(gcov_unsigned_t);
}

void prune_non_dynamic_stacks(gooda::afdo_data& data){
    for(auto& function : data.functions){
        function.stacks.erase(
                std::remove_if(function.stacks.begin(), function.stacks.end(), 
                    [](gooda::afdo_stack& stack){ return stack.num_inst == 0;}), 
                function.stacks.end());
    }
}

/*!
 * \brief Compute the length of each section of the AFDO data file. 
 * \param data the Data file
 */
void compute_lengths(gooda::afdo_data& data){
    //The counts
    ++data.length_file_section;
    ++data.length_function_section;
    ++data.length_modules_section;

    for(auto& file_name : data.file_names){
        data.length_file_section += sizeof_string(file_name);
    }

    for(auto& function : data.functions){
        //function name
        data.length_function_section += sizeof_string(function.name);
        
        //file (1), total_count (2) and entry_count (2)
        data.length_function_section += 5;
        
        //the number of stacks (1)
        data.length_function_section += 1;
        
        //The size for each stack (number of pos (1), count (2), num_inst (2))
        data.length_function_section += 5 * function.stacks.size();

        for(auto& stack : function.stacks){
            //file (1), func (1), line (1), discr (1)
            data.length_function_section += 4 * stack.stack.size();
        }
    }

    for(auto& module : data.modules){
        //Module name
        data.length_modules_section += sizeof_string(module.name);

        //8 unsigned for the data
        data.length_modules_section += 8;

        for(auto& str : module.strings){
            data.length_modules_section += sizeof_string(str);
        }
    }

    //num_counter (1), min_counter (2)
    data.length_working_set_section = data.working_set.size() * 3;
}

void compute_working_set(gooda::afdo_data& data){
    //Fill the working set with zero
    for(auto& working_set : data.working_set){
        working_set.num_counter = 0;
        working_set.min_counter = 0;
    }

    std::map<std::size_t, std::size_t> histogram;
    std::size_t total_count = 0;

    for(auto& function : data.functions){
        for(auto& stack : function.stacks){
            histogram[stack.count] += stack.num_inst;
            total_count += stack.num_inst * stack.count;
        }
    }

    auto rit = histogram.rbegin();
    auto rend = histogram.rend();

    unsigned int bucket_num = 0;
    std::size_t accumulated_count = 0;
    std::size_t accumulated_inst = 0;
    std::size_t one_bucket_count = total_count / (gooda::WS_SIZE + 1);

    while(rit != rend && bucket_num < gooda::WS_SIZE){
        auto count = rit->first;
        auto inst = rit->second;

        while(count * inst + accumulated_count > one_bucket_count * (bucket_num + 1)){
            auto offset = (one_bucket_count * (bucket_num + 1) - accumulated_count) / count;

            accumulated_inst += offset;
            accumulated_count += offset * count;

            inst -= offset;

            data.working_set.at(bucket_num).num_counter = accumulated_inst;
            data.working_set.at(bucket_num).min_counter = count;
            ++bucket_num;
        }

        accumulated_inst += inst;
        accumulated_count += inst * count;

        ++rit;
    }
}

std::string get_application_file(const gooda::gooda_report& report){
    auto& line = report.hotspot_function(0);
    auto application_file = line.get_string(report.get_hotspot_file().column(MODULE));

    log::emit<log::Debug>() << "Found application file in \"" << application_file << "\"" << log::endl;

    return application_file;
}

} //End of anonymous namespace

void gooda::read_report(const gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    bool lbr = vm.count("lbr");

    data.application_file = get_application_file(report);

    //Choose the correct counter
    std::string counter_name;
    if(lbr){
        counter_name = BB_EXEC;
    } else {
        counter_name = UNHALTED_CORE_CYCLES;
    }

    //The set of basic blocks of each function
    std::map<std::size_t, bb_vector> basic_blocks;

    for(std::size_t i = 0; i < report.functions(); ++i){
        auto& line = report.hotspot_function(i);

        auto string_cycles = line.get_string(report.get_hotspot_file().column(counter_name));

        //Some functions are filled empty by Gooda for some reason
        //In some case, it means 0, in that case, it is not a problem to ignore it either, cause not really hotspot
        if(string_cycles.empty()){
            continue;
        }
    
        gooda::afdo_function function;
        function.name = line.get_string(report.get_hotspot_file().column(FUNCTION_NAME));
        function.file = "unknown"; //The file will be filled by read_asm
        function.total_count = line.get_counter(report.get_hotspot_file().column(counter_name));
        function.i = i;

        data.add_file_name(function.file);
        data.add_file_name(function.name);
        
        //Collect function.file and function.entry_count
        auto bbs = collect_basic_blocks(report, data, function, counter_name);
        
        if(!function.valid){
            continue;
        }

        if(lbr){
            lbr_annotate(report, data, function, bbs);
        } else {
            ca_annotate(report, data, function, bbs);
        }

        data.functions.push_back(std::move(function));
    }

    prune_non_dynamic_stacks(data);

    compute_working_set(data);
    compute_lengths(data);

    //Note: No need to fill the modules because it is not used by GCC
    //It will be automatically written empty by the AFDO generator
}
