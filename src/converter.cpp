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
#include <unordered_set>
#include <utility>
#include <chrono>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "assert.hpp"
#include "converter.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "hash.hpp"

//Chrono typedefs
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds milliseconds;

typedef std::pair<std::string, long> inlined_key;

namespace {

//Common utilities

//The inlining cache contains the inline stack for each inlined point
std::unordered_map<inlined_key, std::vector<gooda::afdo_pos>> inlining_cache;

//The discriminator cache contains the discriminator for each address
std::unordered_map<inlined_key, gcov_unsigned_t> discriminator_cache;

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

typedef std::vector<gooda_bb> bb_vector;

gooda::afdo_stack& get_stack(gooda::afdo_function& function, std::string func, std::string file, std::size_t line, std::size_t discriminator){
    for(auto& stack : function.stacks){
        if(stack.stack.size() == 1){
            auto& pos = stack.stack.front();
            if(pos.line == line && pos.func == func && pos.file == file && pos.discriminator == discriminator){
                return stack;
            }
        }
    }
    
    gooda::afdo_stack stack;

    stack.stack.emplace_back(func, file, line, discriminator);

    function.stacks.push_back(std::move(stack));

    return function.stacks.back(); 
}

long get_min_address(bb_vector& block_set){
    long start_address = std::numeric_limits<long>::max();
    
    for(auto& block : block_set){
        start_address = std::min(start_address, block.address);
    }

    return start_address;
}

std::string get_function_name(const std::string& application_file, bb_vector& block_set){
    auto start_address = get_min_address(block_set);

    auto key = std::make_pair(application_file, start_address);

    //If the file does not exist, the cache will not be filled
    //It can also come from an error of addr2line
    if(inlining_cache.find(key) == inlining_cache.end()){
        log::emit<log::Debug>() << application_file << ":" << start_address << " not in cache" << log::endl;

        return "";
    }

    //TODO Change that
    return inlining_cache[key].front().func;
}

gooda::afdo_stack& get_inlined_stack(gooda::afdo_function& function, std::string src_func, std::string src_file, std::size_t src_line, std::size_t src_discriminator,
            std::string dest_func, std::string dest_file, std::size_t dest_line, std::size_t dest_discriminator){
    for(auto& stack : function.stacks){
        if(stack.stack.size() == 2){
            auto& src_pos = stack.stack.front();

            if(src_pos.line == src_line && src_pos.func == src_func && src_pos.file == src_file && src_pos.discriminator == src_discriminator){
                auto& dest_pos = stack.stack.back();
            
                if(dest_pos.line == dest_line && dest_pos.func == dest_func && dest_pos.file == dest_file && dest_pos.discriminator == dest_discriminator){
                    return stack;
                }
            }
        }
    }
    
    gooda::afdo_stack stack;

    //Source position
    stack.stack.emplace_back(src_func, src_file, src_line, src_discriminator);
    
    //Destination position
    stack.stack.emplace_back(dest_func, dest_file, dest_line, dest_discriminator);

    function.stacks.push_back(std::move(stack));

    return function.stacks.back(); 
}

gooda::afdo_stack& get_inlined_stack(gooda::afdo_function& function, long address){
    auto key = std::make_pair(function.executable_file, address);

    //If the file does not exist, the cache will not be filled
    //It can also come from an error of addr2line
    if(inlining_cache.find(key) == inlining_cache.end()){
        log::emit<log::Warning>() << function.executable_file << ":" << address << " not in cache" << log::endl;

        function.stacks.push_back({});

        return function.stacks.back(); 
    }

    auto& vector = inlining_cache[key];
    std::reverse(vector.begin(), vector.end());

    //Try to find an existing equivalent stack

    for(auto& stack : function.stacks){
        if(stack.stack.size() == vector.size()){
            bool equals = true;
            for(std::size_t i = 0; i < vector.size(); ++i){
                if(stack.stack[i] != vector[i]){
                    equals = false;
                    break;
                }
            }

            if(equals){
                return stack;
            }
        }
    }

    //If its not found, create a new stack
    
    gooda::afdo_stack new_stack;

    for(auto& pos : vector){
        new_stack.stack.emplace_back(std::move(pos));
    }

    function.stacks.push_back(std::move(new_stack));

    return function.stacks.back(); 
}

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
    
    return std::make_pair(std::move(normal_blocks), std::move(inlined_block_sets));
}

bb_vector collect_basic_blocks(const gooda::gooda_report& report, gooda::afdo_function& function, bool lbr){
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
                block.address = line.get_address(file.column(ADDRESS));
                block.gooda_function = function.i;
                
                if(lbr){
                    block.exec_count = line.get_counter(file.column(BB_EXEC));
                }

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
            
            //Get the entry basic block and the function file
            if(boost::starts_with(disassembly, "Basic Block 1 <")){
                if(lbr){
                    function.entry_count = line.get_counter(file.column(BB_EXEC));
                } else {
                    auto count = file.multiplex_line().get_double(file.column(UNHALTED_CORE_CYCLES)) * line.get_counter(file.column(UNHALTED_CORE_CYCLES));
                    function.entry_count = static_cast<gcov_type>(count);
                }

                bb_found = true;
            } else if(bb_found){
                function.file = line.get_string(file.column(PRINC_FILE));

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

void ca_annotate(const gooda::gooda_report& report, gooda::afdo_function& function, bb_vector& basic_blocks){
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
                auto discriminator = discriminator_cache[{function.executable_file, asm_line.get_address(asm_file.column(ADDRESS))}];

                auto& stack = get_stack(function, function.name, function.file, line_number, discriminator);

                auto count = asm_file.multiplex_line().get_double(asm_file.column(UNHALTED_CORE_CYCLES)) * asm_line.get_counter(asm_file.column(UNHALTED_CORE_CYCLES));
                stack.count = std::max(stack.count, static_cast<gcov_type>(count));

                auto cache_misses = asm_file.multiplex_line().get_double(asm_file.column(LOAD_LATENCY)) * asm_line.get_counter(asm_file.column(LOAD_LATENCY));
                stack.cache_misses = std::max(stack.cache_misses, static_cast<gcov_type>(cache_misses));
                
                //There is one more dynamic instruction
                ++stack.num_inst;
            }
        }
        
        //2. Handle inlined blocks if any
        for(auto& block_set : inlined_block_sets){
            gooda_assert(block_set.size() > 0, "Something went wrong with BB Collection");

            for(auto& block : block_set){
                for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
                    gooda_assert(j < asm_file.lines(), "Something went wrong with BB collection");

                    auto& asm_line = asm_file.line(j);
                    auto line_number = asm_line.get_counter(asm_file.column(PRINC_LINE));
                    auto discriminator = discriminator_cache[{function.executable_file, asm_line.get_address(asm_file.column(ADDRESS))}];

                    //It is possible that a basic block is not made only 
                    //of inlined lines
                    if(asm_line.get_string(asm_file.column(INIT_FILE)).empty()){
                        auto& stack = get_stack(function, function.name, function.file, line_number, discriminator); 

                        auto count = asm_file.multiplex_line().get_double(asm_file.column(UNHALTED_CORE_CYCLES)) * asm_line.get_counter(asm_file.column(UNHALTED_CORE_CYCLES));
                        stack.count = std::max(stack.count, static_cast<gcov_type>(count));

                        auto cache_misses = asm_file.multiplex_line().get_double(asm_file.column(LOAD_LATENCY)) * asm_line.get_counter(asm_file.column(LOAD_LATENCY));
                        stack.cache_misses = std::max(stack.cache_misses, static_cast<gcov_type>(cache_misses));

                        //There is one more dynamic instruction
                        ++stack.num_inst;
                    } else {
                        auto callee_function_name = get_function_name(function.executable_file, block_set);
                        auto callee_line_number = asm_line.get_counter(asm_file.column(INIT_LINE));

                        auto& stack = get_inlined_stack(
                                function, 
                                function.name, function.file, line_number, discriminator,
                                callee_function_name, block.inlined_file, callee_line_number, 0); 

                        auto count = asm_file.multiplex_line().get_double(asm_file.column(UNHALTED_CORE_CYCLES)) * asm_line.get_counter(asm_file.column(UNHALTED_CORE_CYCLES));
                        stack.count = std::max(stack.count, static_cast<gcov_type>(count));

                        auto cache_misses = asm_file.multiplex_line().get_double(asm_file.column(LOAD_LATENCY)) * asm_line.get_counter(asm_file.column(LOAD_LATENCY));
                        stack.cache_misses = std::max(stack.cache_misses, static_cast<gcov_type>(cache_misses));
                        
                        //There is one more dynamic instruction
                        ++stack.num_inst;
                    }
                }
            }
        }
    }
}

//LBR Mode

void lbr_annotate(const gooda::gooda_report& report, gooda::afdo_function& function, bb_vector& basic_blocks){
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
                auto discriminator = discriminator_cache[{function.executable_file, asm_line.get_address(asm_file.column(ADDRESS))}];

                auto& stack = get_stack(function, function.name, function.file, line_number, discriminator);
                stack.count = std::max(stack.count, block.exec_count);
                ++stack.num_inst;
            }
        }

        //2. Handle inlined blocks if any
        if(!inlined_block_sets.empty()){
            for(auto& block_set : inlined_block_sets){
                gooda_assert(block_set.size() > 0, "Something went wrong with BB Collection");

                for(auto& block : block_set){
                    for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
                        gooda_assert(j < asm_file.lines(), "Something went wrong with BB collection");

                        auto& asm_line = asm_file.line(j);
                        auto line_number = asm_line.get_counter(asm_file.column(PRINC_LINE));
                        auto discriminator = discriminator_cache[{function.executable_file, asm_line.get_address(asm_file.column(ADDRESS))}];

                        //It is possible that a basic block is not made only 
                        //of inlined lines
                        if(asm_line.get_string(asm_file.column(INIT_FILE)).empty()){
                            auto& stack = get_stack(function, function.name, function.file, line_number, discriminator); 

                            stack.count = std::max(stack.count, block.exec_count);
                            ++stack.num_inst;
                        } else {
                            auto& stack = get_inlined_stack(function, asm_line.get_address(asm_file.column(ADDRESS)));

                            stack.count = std::max(stack.count, block.exec_count);
                            ++stack.num_inst;
                        }
                    }
                }
            }
        }
    }
}

/*!
 * \brief Return the AFDO size of the given string
 * \param str The string to get the size of.
 * \return The AFDO size of the given string.
 */
unsigned int sizeof_string(const std::string& str){
    return 1 + (str.length() + sizeof(gcov_unsigned_t)) / sizeof(gcov_unsigned_t);
}

/*!
 * \brief Remove all the stacks that have no corresponding dynamic instructions
 * \param data The AFDO profile
 */
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

/*!
 * \brief Compute the working set for the given data. 
 * \param data the AFDO profile
 */
void compute_working_set(gooda::afdo_data& data, boost::program_options::variables_map& vm){
    //Fill the working set with zero
    for(auto& working_set : data.working_set){
        working_set.num_counter = 0;
        working_set.min_counter = 0;
    }

    //Let it be zero if the user does not want it
    if(vm.count("nows")){
        return;
    }

    std::map<std::size_t, std::size_t> histogram;
    std::size_t total_count = 0;
    
    for(auto& function : data.functions){
        for(auto& stack : function.stacks){
            histogram[stack.count]++;
            total_count += stack.count;
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
        auto num_inst = rit->second;

        while(count * num_inst + accumulated_count > one_bucket_count * (bucket_num + 1)){
            int offset = (one_bucket_count * (bucket_num + 1) - accumulated_count) / count;

            accumulated_inst += offset;
            accumulated_count += offset * count;

            num_inst -= offset;

            data.working_set.at(bucket_num).num_counter = accumulated_inst;
            data.working_set.at(bucket_num).min_counter = count;
            ++bucket_num;
        }

        accumulated_inst += num_inst;
        accumulated_count += num_inst * count;

        ++rit;
    }
}

/*!
 * \brief Return the application executable file of the function i
 * \param report The Gooda report
 * \param i The index of the function to get the executable from
 * \return The ELF file the function is located in. 
 */
std::string get_application_file(const gooda::gooda_report& report, std::size_t i){
    auto& line = report.hotspot_function(i);
    auto application_file = line.get_string(report.get_hotspot_file().column(MODULE));

    log::emit<log::Debug>() << "Found application file in \"" << application_file << "\"" << log::endl;

    return application_file;
}

/*!
 * \brief Return the process filter
 * \param report the report to fill. 
 * \param vm The configuration. 
 * \param counter_name The name of the counter. 
 * \return the process filter
 */
std::string get_process_filter(const gooda::gooda_report& report, boost::program_options::variables_map& vm, std::string& counter_name){
    if(vm.count("filter")){
        std::string max_process = "";
        std::size_t max_value = 0;

        for(std::size_t i = 0; i < report.processes(); ++i){
            auto& line = report.process(i);

            auto process = line.get_string(report.get_process_file().column(PROCESS_PATH));

            //The summary of the process view
            if(process == "Global sample breakdown"){
                continue;
            }

            auto value = line.get_counter(report.get_process_file().column(counter_name));

            if(value > max_value){
                max_value = value;
                max_process = process;
            }
        }

        return max_process;
    } else if(vm.count("process")){
        auto filter = vm["process"].as<std::string>();

        return filter;
    } else {
        return "";
    }
}

/*!
 * \brief Fill the inlining cache
 * \param report The gooda report to fill
 * \param data The data already filled
 * \param maps The indexes map
 * \param vm The configuration
 */
void fill_inlining_cache(const gooda::gooda_report& report, gooda::afdo_data& data, std::vector<long>& maps, boost::program_options::variables_map& vm){
    std::unordered_map<std::string, std::vector<std::size_t>> addresses;
    
    //Collect the inlined addresses

    for(std::size_t i = 0; i < report.functions(); ++i){
        if(maps.at(i) >= 0){
            auto& function = data.functions.at(maps.at(i));
            auto& file = report.asm_file(function.i);

            for(std::size_t j = 0; j < file.lines(); ++j){
                auto& line = file.line(j);

                if(!line.get_string(file.column(INIT_LINE)).empty()){
                    auto address = line.get_address(file.column(ADDRESS));
                    addresses[function.executable_file].push_back(address);
                }
            }
        }
    }

    //Fill the inlining cache by using addr2line

    for(auto& address_set : addresses){
        if(!gooda::exists(address_set.first)){
            log::emit<log::Warning>() << "File " << address_set.first << " does not exist" << log::endl;

            continue;
        }

        log::emit<log::Debug>() << "Query " << address_set.first << " with " << vm["addr2line"].as<std::string>() << log::endl;

        std::stringstream ss;
        ss << vm["addr2line"].as<std::string>() << " -f -a -i --exe=" << address_set.first << " ";

        for(auto& address : address_set.second){
            ss << "0x" << std::hex << address << " ";
        }

        auto command = ss.str();
        auto result = gooda::exec_command_result(command);
        log::emit<log::Trace>() << "Run command \"" << command << "\"" << log::endl;

        std::istringstream result_stream(result);
        std::string str_line;    

        std::size_t address = 0;

        while (std::getline(result_stream, str_line)) {
            if(boost::starts_with(str_line, "0x000000")){
                std::istringstream convert(str_line);
                convert >> std::hex >> address;
            } else {
                auto key = std::make_pair(address_set.first, address);

                auto function_name = str_line;

                std::getline(result_stream, str_line);

                std::string file_name;
                std::string line_number;
                gcov_unsigned_t discriminator;
                
                auto start_disc = str_line.find("(discriminator ");
                auto start_number = str_line.rfind(":");
                auto start_file = str_line.rfind("/");

                if(start_disc == std::string::npos){
                    line_number = str_line.substr(start_number + 1, str_line.size() - start_number - 1);
                    file_name = str_line.substr(start_file + 1, start_number - start_file - 1);
                    discriminator = 0;
                } else {
                    line_number = str_line.substr(start_number + 1, start_disc - start_number - 2);
                    file_name = str_line.substr(start_file + 1, start_number - start_file - 1);

                    auto end = str_line.find(")", start_disc); 
                    auto discriminator_str = str_line.substr(start_disc + 15, end - start_disc - 15);
                    discriminator = boost::lexical_cast<gcov_unsigned_t>(discriminator_str);
                }
                
                inlining_cache[key].emplace_back(function_name, file_name, boost::lexical_cast<gcov_unsigned_t>(line_number), discriminator);
            }
        }
    }
}

/*!
 * \brief Fill the discriminator cache
 * \param report The gooda report to fill
 * \param data The data already filled
 * \param maps The indexes map
 * \param vm The configuration
 */
void fill_discriminator_cache(const gooda::gooda_report& report, gooda::afdo_data& data, std::vector<long>& maps, boost::program_options::variables_map& vm){
    if(vm.count("discriminators")){
        std::unordered_map<std::string, std::vector<std::size_t>> asm_addresses;

        for(std::size_t i = 0; i < report.functions(); ++i){
            if(maps.at(i) >= 0){
                auto& function = data.functions.at(maps.at(i));
                auto& file = report.asm_file(function.i);

                for(std::size_t j = 0; j < file.lines(); ++j){
                    auto& line = file.line(j);

                    if(!line.get_string(file.column(ADDRESS)).empty()){
                        asm_addresses[function.executable_file].push_back(line.get_address(file.column(ADDRESS)));
                    }
                }
            }
        }

        for(auto& address_set : asm_addresses){
            if(!gooda::exists(address_set.first)){
                log::emit<log::Warning>() << "File " << address_set.first << " does not exist" << log::endl;

                continue;
            }

            log::emit<log::Debug>() << "Discriminator Query " << address_set.first << " with " << vm["addr2line"].as<std::string>() << log::endl;

            std::stringstream ss;
            ss << vm["addr2line"].as<std::string>() << " -a --exe=" << address_set.first << " ";

            for(auto& address : address_set.second){
                ss << "0x" << std::hex << address << " ";
            }

            auto command = ss.str();
            auto result = gooda::exec_command_result(command);
            log::emit<log::Trace>() << "Run command \"" << command << "\"" << log::endl;

            std::istringstream result_stream(result);
            std::string str_line;    
            bool next = false;

            std::size_t address = 0;

            while (std::getline(result_stream, str_line)) {
                if(boost::starts_with(str_line, "0x000000")){
                    std::istringstream convert(str_line);
                    convert >> std::hex >> address;
                    next = true;
                } else if(next){
                    auto key = std::make_pair(address_set.first, address);

                    auto search = str_line.find("(discriminator ");
                    if(search == std::string::npos){
                        discriminator_cache[key] = 0;    
                    } else {
                        auto end = str_line.find(")", search); 
                        auto discriminator = str_line.substr(search + 15, end - search - 15);
                        discriminator_cache[key] = boost::lexical_cast<gcov_unsigned_t>(discriminator);    
                    }

                    next = false;
                }
            }
        }
    }
}

/*!
 * \brief Update the function names to use the mangled names. 
 * \param report The gooda report to fill
 * \param data The data already filled
 * \param maps The indexes map
 * \param vm The configuration
 */
void update_function_names(const gooda::gooda_report& report, gooda::afdo_data& data, std::vector<long>& maps, boost::program_options::variables_map& vm){
    std::unordered_map<std::string, std::vector<std::string>> asm_addresses;
    std::unordered_map<std::pair<std::string, std::string>, std::string> mangled_names;
    std::unordered_map<std::size_t, std::pair<std::string, std::string>> function_addresses;

    //Collect one address for each function

    for(std::size_t i = 0; i < report.functions(); ++i){
        if(maps.at(i) >= 0){
            auto& function = data.functions.at(maps.at(i));
            auto& file = report.asm_file(function.i);

            //Get the first non empty address and put it on the map
            for(std::size_t j = 0; j < file.lines(); ++j){
                auto& line = file.line(j);

                auto address = line.get_string(file.column(ADDRESS));

                if(!address.empty()){
                    function_addresses[function.i] = {function.executable_file, address};
                    asm_addresses[function.executable_file].emplace_back(std::move(address));
                    break;
                }
            }
        }
    }

    //Collect the mangled function names

    for(auto& address_set : asm_addresses){
        if(!gooda::exists(address_set.first)){
            log::emit<log::Warning>() << "File " << address_set.first << " does not exist" << log::endl;

            continue;
        }

        log::emit<log::Debug>() << "Mangled Query " << address_set.first << " with " << vm["addr2line"].as<std::string>() << log::endl;

        std::stringstream ss;
        ss << vm["addr2line"].as<std::string>() << " -a -f --exe=" << address_set.first << " ";

        for(auto& address : address_set.second){
            ss << address << " ";
        }

        auto command = ss.str();
        auto result = gooda::exec_command_result(command);
        log::emit<log::Trace>() << "Run command \"" << command << "\"" << log::endl;

        std::istringstream result_stream(result);
        std::string str_line;    
        bool next = false;

        std::string address;

        while (std::getline(result_stream, str_line)) {
            if(boost::starts_with(str_line, "0x000000")){
                address = "0x";

                int i = 2;
                while(str_line[i] == '0'){
                    ++i;
                }

                address += str_line.substr(i, str_line.size() - i);

                next = true;
            } else if(next){
                mangled_names[{address_set.first, address}] = str_line;

                next = false;
            }
        }
    }

    //Give the functions their names
    
    for(std::size_t i = 0; i < report.functions(); ++i){
        if(maps.at(i) >= 0){
            auto& function = data.functions.at(maps.at(i));

            function.name = mangled_names[function_addresses[function.i]];
        }
    }
}

void fill_file_name_table(gooda::afdo_data& data){
    for(auto& function : data.functions) {
        data.add_file_name(function.name);
        data.add_file_name(function.file);

        for(auto& stack : function.stacks){
            for(auto& pos : stack.stack){
                data.add_file_name(pos.file);
                data.add_file_name(pos.func);
            }
        }
    }
}

} //End of anonymous namespace

void gooda::convert_to_afdo(const gooda::gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    bool lbr = vm.count("lbr");

    //Choose the correct counter
    std::string counter_name = lbr ? BB_EXEC : UNHALTED_CORE_CYCLES;

    auto filter = get_process_filter(report, vm, counter_name);
    log::emit<log::Debug>() << "Filter by \"" << filter << "\"" << log::endl;

    //Maps index of report.functions() to index of data.functions. -1 indicates that there is no function
    std::vector<long> maps(report.functions());
    std::fill(maps.begin(), maps.end(), -1);

    for(std::size_t i = 0; i < report.functions(); ++i){
        auto& line = report.hotspot_function(i);

        //Only if the function passes the filters
        if(filter.empty() || line.get_string(report.get_hotspot_file().column(PROCESS)) == filter){
            auto string_cycles = line.get_string(report.get_hotspot_file().column(counter_name));

            //Some functions are filled empty by Gooda for some reason
            //In some case, it means 0, in that case, it is not a problem to ignore it either, cause not really hotspot
            if(string_cycles.empty()){
                continue;
            }

            gooda::afdo_function function;
            function.i = i;
            function.executable_file = get_application_file(report, i);
            function.name = line.get_string(report.get_hotspot_file().column(FUNCTION_NAME));

            if(lbr){
                function.total_count = line.get_counter(report.get_hotspot_file().column(SW_INST_RETIRED));
            } else {
                auto count = 
                        report.get_hotspot_file().multiplex_line().get_double(report.get_hotspot_file().column(UNHALTED_CORE_CYCLES)) 
                      * line.get_counter(report.get_hotspot_file().column(UNHALTED_CORE_CYCLES));

                function.total_count = static_cast<gcov_type>(count);
            }

            //We need the asm file to continue 
            
            if(!report.has_asm_file(function.i)){
                continue;
            }
            
            //Check that the function file is correctly set

            auto& file = report.asm_file(function.i);

            bool invalid = false;

            for(std::size_t j = 0; j < file.lines(); ++j){
                auto& line = file.line(j);

                //Basic Block have no file
                if(boost::starts_with(line.get_string(file.column(DISASSEMBLY)), "Basic Block")){
                    continue;
                }

                //Line without addresses have special meaning
                if(line.get_string(file.column(ADDRESS)).empty()){
                    continue;
                }

                //Gooda does not always found the source file of a function
                //In that case, declare the function as invalid and return quickly
                auto princ_file = line.get_string(file.column(PRINC_FILE));
                if(princ_file == "null"){
                    log::emit<log::Warning>() << function.name << " is invalid (null file)" << log::endl;

                    invalid = true;
                    break;
                }

                if(princ_file.empty()){
                    log::emit<log::Warning>() << function.name << " is invalid (empty file)" << log::endl;

                    invalid = true;
                    break;
                }
            }

            if(invalid){
                continue;
            }

            //Add the function

            maps[i] = data.functions.size();
            data.functions.push_back(std::move(function));
        }
    }

    //Update function names (replace unmangled with mangled names)
    update_function_names(report, data, maps, vm);

    //Fill the inlining cache (gets inlined function names)
    fill_inlining_cache(report, data, maps, vm);
    
    //Fill the discriminator cache (gets the discriminators of each lines)
    fill_discriminator_cache(report, data, maps, vm);

    //Generate the inline stacks

    for(std::size_t i = 0; i < report.functions(); ++i){
        if(maps.at(i) >= 0){
            auto& function = data.functions.at(maps.at(i));

            //Collect function.file and function.entry_count
            auto bbs = collect_basic_blocks(report, function, lbr);

            if(lbr){
                lbr_annotate(report, function, bbs);
            } else {
                ca_annotate(report, function, bbs);
            }
        }
    }

    //Remove all the stacks that have no dynamic instructions
    prune_non_dynamic_stacks(data);

    //Fill the file name table with the strings from the AFDO profile
    fill_file_name_table(data);

    //Compute the working set
    compute_working_set(data, vm);

    //Set the sizes of the different sections
    compute_lengths(data);

    //Note: No need to fill the modules because it is not used by GCC
    //It will be automatically written empty by the AFDO generator
}
