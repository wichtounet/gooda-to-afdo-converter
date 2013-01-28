//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file converter.cpp
 * \brief Implementation of the conversion from Gooda spreadsheets to AFDO profile.
 */

#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "assert.hpp"
#include "converter.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "hash.hpp"
#include "gooda_exception.hpp"

namespace {

/*!
 * \typedef address_key
 * \brief Identifies an instruction inside an ELF file by its address
 */
typedef std::pair<std::string, std::string> address_key;

std::unordered_map<address_key, std::vector<gooda::afdo_pos>> inlining_cache;   //!< Inlining stack cache

std::unordered_map<address_key, gcov_unsigned_t> discriminator_cache;           //!< Discriminator cache

/*!
 * \struct gooda_bb
 * \brief A basic block extracted from the assembly view
 */
struct gooda_bb {
    unsigned long exec_count;       //!< The number of executions of the BB (only in LBR)
    std::size_t gooda_line_start;   //!< The first line (in the assembly spreadsheet)
    std::size_t gooda_line_end;     //!< The last line (in the assembly spreadsheet)
};

/*!
 * \typedef bb_vector
 * \brief A vector of basic block
 */
typedef std::vector<gooda_bb> bb_vector;

/*!
 * \brief Get an inline stack for the given position. 
 *
 * If the inline stack already exists, a reference to it is returned, else a new one is created.
 *
 * \param function The AFDO function
 * \param position The afdo position to search for
 * \return A reference to the corresponding inline stack
 */
gooda::afdo_stack& get_stack(gooda::afdo_function& function, gooda::afdo_pos&& position){
    //Try to find an equivalent stack

    for(auto& stack : function.stacks){
        if(stack.stack.size() == 1){
            if(stack.stack.front() == position){
                return stack;
            }
        }
    }

    //If not found, create a new stack
    
    gooda::afdo_stack stack;

    stack.stack.push_back(std::move(position));

    function.stacks.push_back(std::move(stack));

    return function.stacks.back(); 
}

gooda::afdo_stack fake_stack; //!< A fake stack used to return an empty stack

/*!
 * \brief Get an inline stack for the given address that is coming from an inlined function
 * \param function The AFDO function
 * \param address The address of the instruction
 * \return A reference to the corresponding inline stack
 */
gooda::afdo_stack& get_inlined_stack(gooda::afdo_function& function, std::string address){
    auto key = std::make_pair(function.executable_file, address);

    //If the file does not exist, the cache will not be filled
    //It can also come from an error of addr2line
    if(inlining_cache.find(key) == inlining_cache.end()){
        log::emit<log::Warning>() << function.executable_file << ":" << address << " not in inlining cache" << log::endl;

        return fake_stack; 
    }

    auto& vector = inlining_cache[key];

    if(vector.empty()){
        log::emit<log::Warning>() << function.executable_file << ":" << address << " indicated an empty inline stack" << log::endl;

        return fake_stack; 
    }

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
    new_stack.stack.reserve(vector.size());

    for(auto& pos : vector){
        new_stack.stack.push_back(std::move(pos));
    }

    function.stacks.push_back(std::move(new_stack));

    return function.stacks.back(); 
}

/*!
 * \brief Collect basic blocks of the given function
 * \param report The Gooda source report
 * \param function The AFDO function
 * \param lbr Indicate if lbr is activated or not
 * \return A vector containing all the basic blocks of the function
 */
bb_vector collect_basic_blocks(const gooda::gooda_report& report, gooda::afdo_function& function, bool lbr){
    bb_vector basic_blocks;
    basic_blocks.reserve(10);

    auto& file = report.asm_file(function.i);

    //Compute the addresses of the first and the last instructions
    auto start_instruction = report.hotspot_function(function.i).get_address(report.get_hotspot_file().column(OFFSET));
    auto length = report.hotspot_function(function.i).get_address(report.get_hotspot_file().column(LENGTH));
    auto last_instruction = start_instruction + length;
    bool bb_found = false;

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

            basic_blocks.push_back(std::move(block));
        } 

        //Get the entry basic block and the function file
        if(boost::starts_with(disassembly, "Basic Block 1 ")){
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
        if(address != start_instruction && address >= last_instruction){
            break;
        }
    }

    gooda_assert(!function.file.empty(), "The function file must be set");

    return basic_blocks;
}

/*!
 * \brief Annotate the function with Unhalted Core Cycles counters
 * \param report The Gooda source report
 * \param function The AFDO function
 * \param basic_blocks The basic blocks
 */
void ca_annotate(const gooda::gooda_report& report, gooda::afdo_function& function, bb_vector& basic_blocks){
    auto& asm_file = report.asm_file(function.i);

    for(auto& block : basic_blocks){
        for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
            gooda_assert(j < asm_file.lines(), "Something went wrong with BB collection");

            auto& asm_line = asm_file.line(j);
            gcov_unsigned_t line_number = asm_line.get_counter(asm_file.column(PRINC_LINE));
            auto discriminator = discriminator_cache[{function.executable_file, asm_line.get_string(asm_file.column(ADDRESS))}];

            auto& stack = asm_line.get_string(asm_file.column(INIT_FILE)).empty()
                ? get_stack(function, {function.name, function.file, line_number, discriminator}) 
                : get_inlined_stack(function, asm_line.get_string(asm_file.column(ADDRESS)));

            auto count = asm_file.multiplex_line().get_double(asm_file.column(UNHALTED_CORE_CYCLES)) * asm_line.get_counter(asm_file.column(UNHALTED_CORE_CYCLES));
            stack.count += static_cast<gcov_type>(count);

            auto cache_misses = asm_file.multiplex_line().get_double(asm_file.column(LOAD_LATENCY)) * asm_line.get_counter(asm_file.column(LOAD_LATENCY));
            stack.cache_misses = std::max(stack.cache_misses, static_cast<gcov_type>(cache_misses));

            //There is one more dynamic instruction
            ++stack.num_inst;
        }
    }
}

/*!
 * \brief Annotate the function with LBR counters
 * \param report The Gooda source report
 * \param function The AFDO function
 * \param basic_blocks The basic blocks
 */
void lbr_annotate(const gooda::gooda_report& report, gooda::afdo_function& function, bb_vector& basic_blocks){
    auto& asm_file = report.asm_file(function.i);

    for(auto& block : basic_blocks){
        for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
            gooda_assert(j < asm_file.lines(), "Something went wrong with BB collection");

            auto& asm_line = asm_file.line(j);
            gcov_unsigned_t line_number = asm_line.get_counter(asm_file.column(PRINC_LINE));
            auto discriminator = discriminator_cache[{function.executable_file, asm_line.get_string(asm_file.column(ADDRESS))}];

            auto& stack = asm_line.get_string(asm_file.column(INIT_FILE)).empty()
                ? get_stack(function, {function.name, function.file, line_number, discriminator}) 
                : get_inlined_stack(function, asm_line.get_string(asm_file.column(ADDRESS)));

            stack.count = std::max(stack.count, block.exec_count);

            //There is one more dynamic instruction
            ++stack.num_inst;
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
 * \param report The Gooda report
 * \param data the AFDO profile
 * \param vm The configuration
 * \param lbr Indicate the mode. 
 */
void compute_working_set(const gooda::gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm, bool lbr){
    //Fill the working set with zero
    for(auto& working_set : data.working_set){
        working_set.num_counter = 0;
        working_set.min_counter = 0;
    }

    //Let it be zero if the user does not want it
    if(vm.count("nows")){
        return;
    }

    std::map<uint64_t, uint64_t> histogram;
    uint64_t total_count = 0;

    auto counter = lbr ? SW_INST_RETIRED : UNHALTED_CORE_CYCLES;
    
    for(auto& function : data.functions){
        auto& asm_file = report.asm_file(function.i);

        for(std::size_t j = 0; j < asm_file.lines(); ++j){
            auto& asm_line = asm_file.line(j);

            if(!asm_line.get_string(asm_file.column(ADDRESS)).empty() && !boost::starts_with(asm_line.get_string(asm_file.column(DISASSEMBLY)), "Basic Block")){
                auto count = asm_file.multiplex_line().get_double(asm_file.column(counter)) * asm_line.get_counter(asm_file.column(counter));

                histogram[count]++;
                total_count += count;
            }
        }
    }

    auto rit = histogram.rbegin();
    auto rend = histogram.rend();

    unsigned int bucket_num = 0;
    uint64_t accumulated_count = 0;
    uint64_t accumulated_inst = 0;
    uint64_t one_bucket_count = total_count / (gooda::WS_SIZE + 1);

    while(rit != rend && bucket_num < gooda::WS_SIZE){
        uint64_t count = rit->first;
        uint64_t num_inst = rit->second;

        while(count * num_inst + accumulated_count > one_bucket_count * (bucket_num + 1)){
            int offset = (one_bucket_count * (bucket_num + 1) - accumulated_count) / count;

            accumulated_inst += offset;
            accumulated_count += offset * count;

            num_inst -= offset;

            if(bucket_num >= gooda::WS_SIZE){
                break;
            }

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
    return report.hotspot_function(i).get_string(report.get_hotspot_file().column(MODULE));
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
        return vm["process"].as<std::string>();
    } else {
        return "";
    }
}

/*!
 * \brief Extract the address from a line coming from addr2line
 * \param str_line The line to parse
 * \return The address is Gooda format
 */
std::string extract_address(const std::string& str_line){
    std::string address = "0x";

    int i = 2;
    while(str_line[i] == '0'){
        ++i;
    }

    address += str_line.substr(i, str_line.size() - i);

    return address;
}

/*!
 * \brief Fill the inlining cache
 * \param report The gooda report to fill
 * \param data The data already filled
 * \param vm The configuration
 */
void fill_inlining_cache(const gooda::gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    std::unordered_map<std::string, std::vector<std::string>> addresses;
    
    //Collect the inlined addresses

    for(auto& function : data.functions){
        auto& file = report.asm_file(function.i);

        for(std::size_t j = 0; j < file.lines(); ++j){
            auto& line = file.line(j);

            if(!line.get_string(file.column(INIT_LINE)).empty()){
                addresses[function.executable_file].push_back(line.get_string(file.column(ADDRESS)));
            }
        }
    }

    //Fill the inlining cache by using addr2line

    for(auto& address_set : addresses){
        auto file = address_set.first;

        if(!vm["folder"].as<std::string>().empty()){
            file = vm["folder"].as<std::string>() + "/" + file;
        }

        if(!gooda::exists(file)){
            log::emit<log::Warning>() << "File " << file << " does not exist" << log::endl;

            continue;
        }

        log::emit<log::Debug>() << "Query " << file << " with " << vm["addr2line"].as<std::string>() << log::endl;

        std::ofstream address_file;
        address_file.open("addresses", std::ios::out );

        for(auto& address : address_set.second){
            address_file << address << " ";
        }

        address_file << std::endl;

        auto command = vm["addr2line"].as<std::string>() + " -f -a -i --exe=" + file + " @addresses";
        log::emit<log::Trace>() << "Run command \"" << command << "\"" << log::endl;
        auto result = gooda::exec_command_result(command);

        remove("addresses");

        std::istringstream result_stream(result);
        std::string str_line;    

        std::string address;

        while (std::getline(result_stream, str_line)) {
            if(boost::starts_with(str_line, "0x000000")){
                address = extract_address(str_line);
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

                if(line_number != "?"){
                    inlining_cache[key].emplace_back(function_name, file_name, boost::lexical_cast<gcov_unsigned_t>(line_number), discriminator);
                }
            }
        }
    }
   
    //AFDO expects the stack ending with the instruction inside the current function
    //Thus, it is necessary to reverse each stack

    for(auto& inlining_entry : inlining_cache){
        auto& inlining_stack = inlining_entry.second;

        std::reverse(inlining_stack.begin(), inlining_stack.end());
    }

    //There is a bug in addr2line 2.23.1 that gives discriminator for each element of the inlining stack
    //However, only the one from the source is valid. DWARF does not allow discriminators in the inline stack
    //Thus, it is necessary to clear the others lines

    for(auto& inlining_entry : inlining_cache){
        auto& inlining_stack = inlining_entry.second;
        for(std::size_t i = 1; i < inlining_stack.size(); ++i){
            inlining_stack.at(i).discriminator = 0;
        }
    }
}

/*!
 * \brief Fill the discriminator cache
 * \param report The gooda report to fill
 * \param data The data already filled
 * \param vm The configuration
 */
void fill_discriminator_cache(const gooda::gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    if(vm.count("discriminators")){
        std::unordered_map<std::string, std::vector<std::string>> asm_addresses;

        for(auto& function : data.functions){
            auto& file = report.asm_file(function.i);

            for(std::size_t j = 0; j < file.lines(); ++j){
                auto& line = file.line(j);

                auto address = line.get_string(file.column(ADDRESS));
                if(!address.empty() && line.get_string(file.column(INIT_FILE)).empty()){
                    asm_addresses[function.executable_file].push_back(std::move(address));
                }
            }
        }

        for(auto& address_set : asm_addresses){
            auto file = address_set.first;

            if(!vm["folder"].as<std::string>().empty()){
                file = vm["folder"].as<std::string>() + "/" + file;
            }

            if(!gooda::exists(file)){
                log::emit<log::Warning>() << "File " << file << " does not exist" << log::endl;

                continue;
            }

            log::emit<log::Debug>() << "Discriminator Query " << file << " with " << vm["addr2line"].as<std::string>() << log::endl;

            std::ofstream address_file;
            address_file.open("addresses", std::ios::out );

            for(auto& address : address_set.second){
                address_file << address << " ";
            }

            address_file << std::endl;

            auto command = vm["addr2line"].as<std::string>() + " -i -a --exe=" + file + " @addresses";
            log::emit<log::Trace>() << "Run command \"" << command << "\"" << log::endl;
            auto result = gooda::exec_command_result(command);

            remove("addresses");

            std::istringstream result_stream(result);
            std::string str_line;    

            std::string address;

            while (std::getline(result_stream, str_line)) {
                if(boost::starts_with(str_line, "0x000000")){
                    address = extract_address(str_line);
                } else {
                    auto key = std::make_pair(address_set.first, address);

                    auto search = str_line.find("(discriminator ");
                    if(search == std::string::npos){
                        discriminator_cache[key] = 0;    
                    } else {
                        auto end = str_line.find(")", search); 
                        auto discriminator = str_line.substr(search + 15, end - search - 15);
                        discriminator_cache[key] = boost::lexical_cast<gcov_unsigned_t>(discriminator);    
                    }
                }
            }
        }
    }
}

/*!
 * \brief Update the function names to use the mangled names. 
 * \param report The gooda report to fill
 * \param data The data already filled
 * \param vm The configuration
 */
void update_function_names(const gooda::gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    std::unordered_map<std::string, std::vector<std::string>> asm_addresses;
    std::unordered_map<std::pair<std::string, std::string>, std::string> mangled_names;
    std::unordered_map<std::size_t, std::pair<std::string, std::string>> function_addresses;

    //Collect one address for each function
    
    std::size_t cpp_files = 0;

    for(auto& function : data.functions){
        auto& file = report.asm_file(function.i);

        //Get the first non empty address and put it on the map
        for(std::size_t j = 0; j < file.lines(); ++j){
            auto& line = file.line(j);

            auto address = line.get_string(file.column(ADDRESS));
            if(!address.empty() && !boost::starts_with(line.get_string(file.column(DISASSEMBLY)), "Basic Block")){
                function_addresses[function.i] = {function.executable_file, address};
                asm_addresses[function.executable_file].push_back(std::move(address));

                auto princ_file = line.get_string(file.column(PRINC_FILE));
                if(boost::ends_with(princ_file, ".cpp")){
                    ++cpp_files;
                }

                break;
            }
        }
    }
    
    bool cpp = cpp_files > data.functions.size() * 0.5;

    //Collect the mangled function names

    for(auto& address_set : asm_addresses){
        auto file = address_set.first;

        if(!vm["folder"].as<std::string>().empty()){
            file = vm["folder"].as<std::string>() + "/" + file;
        }

        if(!gooda::exists(file)){
            log::emit<log::Warning>() << "File " << file << " does not exist" << log::endl;

            continue;
        }

        log::emit<log::Debug>() << "Mangled Query " << file << " with objdump" << log::endl;

        auto command = "objdump --section=.text --syms " + file;
        auto result = gooda::exec_command_result(command);
        log::emit<log::Trace>() << "Run command \"" << command << "\"" << log::endl;

        std::istringstream result_stream(result);
        std::string str_line;    

        std::string address;

        while (std::getline(result_stream, str_line)) {
            if(boost::starts_with(str_line, "000000")){
                auto address = "0x" + str_line.substr(10, 6);

                std::string sep("              ");
                auto search = str_line.find(sep);

                if(search != std::string::npos){
                    auto function_name = str_line.substr(search + sep.size(), str_line.size() - search - sep.size());
                    mangled_names[{address_set.first, address}] = function_name;
                }
            } 
        }
    }

    //Give the functions their names
   
    for(auto& function : data.functions){
        function.name = mangled_names[function_addresses[function.i]];

        //In C++ mode the name always should always start with underscore
        if(function.name.empty() || (cpp && function.name[0] != '_')){
            log::emit<log::Warning>() << "addr2line reported invalid name for a function: " << function.name << log::endl;
        }
    }
}

/*!
 * \brief Fill the string table of the data
 * \param data The data already filled
 */
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

/*!
 * \brief Remove all functions that have neither count nor a stack with a count. 
 * \param data The data already filled by the previous passes
 */
void prune_uncounted_functions(gooda::afdo_data& data){
    auto it = data.functions.begin();

    while(it != data.functions.end()){
        auto& function = *it;

        if(function.total_count == 0 && function.entry_count == 0){
            bool count = 0;
            for(auto& stack : function.stacks){
                count += stack.count;
            }
            
            if(count == 0){
                it = data.functions.erase(it);
                continue;
            }
        }

        ++it;
    }
}

/*!
 * \brief Return the total count of the given counter in the hotspot function list
 * \param report The gooda report
 * \param counter_name The name of the counter
 * \return The sum of all the values of the given counter. 
 */
std::size_t total_count(const gooda::gooda_report& report, const std::string& counter_name){
    auto& hotspot_file = report.get_hotspot_file();

    std::size_t total = 0;
    for(std::size_t i = 0; i < report.functions(); ++i){
        total += report.hotspot_function(i).get_counter(hotspot_file.column(counter_name));
    }

    return total;
}

} //End of anonymous namespace

void gooda::convert_to_afdo(const gooda::gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    bool lbr;
    if(vm.count("auto")){
        auto total_count_lbr = total_count(report, BB_EXEC); 
        lbr = total_count_lbr > 0;
    } else {
        lbr = vm.count("lbr");
    }

    //Choose the correct counter
    std::string counter_name = lbr ? BB_EXEC : UNHALTED_CORE_CYCLES;

    if(!vm.count("auto")){
        //Verify that the file has the correct column
        if(total_count(report, counter_name) == 0){
            throw gooda::gooda_exception("The file is not valid for the current mode");
        }
    }

    //Empty each cache
    inlining_cache.clear();
    discriminator_cache.clear();

    auto filter = get_process_filter(report, vm, counter_name);
    log::emit<log::Debug>() << "Filter by \"" << filter << "\"" << log::endl;

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

            data.functions.push_back(std::move(function));
        }
    }

    //Update function names (replace unmangled with mangled names)
    update_function_names(report, data, vm);

    //Fill the inlining cache (gets inlined function names)
    fill_inlining_cache(report, data, vm);
    
    //Fill the discriminator cache (gets the discriminators of each lines)
    fill_discriminator_cache(report, data, vm);

    //Generate the inline stacks

    for(auto& function : data.functions){
        //Collect function.file and function.entry_count
        auto bbs = collect_basic_blocks(report, function, lbr);

        if(lbr){
            lbr_annotate(report, function, bbs);
        } else {
            ca_annotate(report, function, bbs);
        }
    }

    //Prune uncounted functions
    prune_uncounted_functions(data);

    //Fill the file name table with the strings from the AFDO profile
    fill_file_name_table(data);

    //Compute the working set
    compute_working_set(report, data, vm, lbr);

    //Set the sizes of the different sections
    compute_lengths(data);

    //Note: No need to fill the modules because it is not used by GCC
    //It will be automatically written empty by the AFDO generator
}
