#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <map>

#include <boost/algorithm/string.hpp>

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

//Normal mode

void read_asm_file(const gooda::gooda_report& report, std::size_t i, gooda::afdo_data& data, const std::string& counter_name){
    if(report.has_asm_file(i)){
        auto& function = data.functions.at(i);
        auto& file = report.asm_file(i);

        //Compute the addresses of the first and the last instructions
        auto start_instruction = report.hotspot_function(i).get_address(report.get_hotspot_file().column(OFFSET));
        auto length = report.hotspot_function(i).get_address(report.get_hotspot_file().column(LENGTH));
        auto last_instruction = start_instruction + length;

        bool bb_found = false;
        bool collection = false;

        function.first_line = std::numeric_limits<decltype(function.first_line)>::max();
        function.last_line = std::numeric_limits<decltype(function.last_line)>::min();

        for(auto& line : file){
            //Gooda does not always found the source file of a function
            //In that case, declare the function as invalid and return quickly
            if(line.get_string(file.column(PRINC_FILE)) == "null"){
                function.valid = false;
                log::emit<log::Warning>() << function.name << " is invalid (null file)" << log::endl;
                return;
            }

            //It indicates the last line, that is not a valid assembly line but a summary of the data
            if(line.get_string(file.column(ADDRESS)).empty()){
                return;
            }

            auto disassembly = line.get_string(file.column(DISASSEMBLY));
            
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

        BOOST_ASSERT_MSG(!function.file.empty(), "The function file must be set");
        BOOST_ASSERT_MSG(function.first_line < std::numeric_limits<decltype(function.first_line)>::max(), "The function first line must be set");
        BOOST_ASSERT_MSG(function.last_line > std::numeric_limits<decltype(function.last_line)>::min(), "The function last line must be set");
    }
}

void read_src_file(const gooda::gooda_report& report, std::size_t i, gooda::afdo_data& data, const std::string& counter_name){
    if(report.has_src_file(i)){
        auto& function = data.functions.at(i);

        auto& file = report.src_file(i);

        for(auto& line : file){
            auto line_number = line.get_counter(file.column(LINE));

            if(line_number >= function.first_line && line_number <= function.last_line){
                gooda::afdo_stack stack;
                stack.count = line.get_counter(file.column(counter_name));
                stack.num_inst = 1; 

                gooda::afdo_pos position;
                position.func = function.name;
                position.file = function.file;
                position.line = line_number;
                position.discr = 0;

                stack.stack.push_back(position);

                function.stacks.push_back(std::move(stack));
            } 
        }
    }
}

//LBR Mode

struct lbr_bb {
    std::string file;
    unsigned long line_start;
    unsigned long exec_count;
    long address;

    std::size_t gooda_function;
    std::size_t gooda_line_start;
    std::size_t gooda_line_end;
    
    unsigned long inlined_line_start;
    std::string inlined_file;
};

std::vector<lbr_bb> collect_bb(const gooda::gooda_report& report, std::size_t i, const std::string& counter_name){
    std::vector<lbr_bb> basic_blocks;

    if(report.has_asm_file(i)){
        auto& file = report.asm_file(i);

        for(std::size_t j = 0; j < file.lines(); ++j){
            auto& line = file.line(j);
            
            auto disassembly = line.get_string(file.column(DISASSEMBLY));
            
            if(boost::starts_with(disassembly, "Basic Block ")){
                lbr_bb block;
                
                block.file = line.get_string(file.column(PRINC_FILE));
                block.line_start = line.get_counter(file.column(PRINC_LINE));
                block.exec_count = line.get_counter(file.column(counter_name));
                block.address = line.get_address(file.column(ADDRESS));
                block.gooda_function = i;

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
        }
    }

    return basic_blocks;
}

std::vector<std::vector<lbr_bb>> compute_inlined_sets(std::vector<lbr_bb> block_set){
    std::unordered_map<inlined_key, std::vector<lbr_bb>> inline_mappings;

    for(auto& block : block_set){
        //If this block comes from an inlined function
        if(!block.inlined_file.empty()){
            inline_mappings[{block.file, block.line_start}].push_back(std::move(block));
        }
    }
    
    std::vector<std::vector<lbr_bb>> inlined_sets;

    for(auto& pair : inline_mappings){
       inlined_sets.push_back(std::move(pair.second)); 
    }

    return inlined_sets;
}

void annotate_src_file(const gooda::gooda_report& report, std::size_t i, gooda::afdo_data& data, std::vector<lbr_bb>& basic_blocks){
    if(report.has_src_file(i)){
        std::vector<lbr_bb> normal_blocks;

        //Extract the
        for(auto& block : basic_blocks){
            if(block.inlined_file.empty()){
                normal_blocks.push_back(std::move(block));
            }
        }

        auto inlined_block_sets = compute_inlined_sets(basic_blocks);

        //Note: From this point basic_blocks contains invalid basic_blocks due to std::moving them
        basic_blocks.clear();

        auto& function = data.functions.at(i);
        auto& file = report.src_file(i);

        //1. Normal pass for non-inlined blocks
        for(auto& line : file){
            auto line_number = line.get_counter(file.column(LINE));

            if(line_number >= function.first_line && line_number <= function.last_line){
                gcov_type counter = 0;

                //Several basic blocks can be on the same line
                //=> Take the max as the value of the line
                for(std::size_t j = 0; j < normal_blocks.size(); ++j){
                    if(
                                (j + 1 < normal_blocks.size() && line_number >= normal_blocks.at(j).line_start && line_number < normal_blocks.at(j + 1).line_start)
                            ||  (j + 1 == normal_blocks.size() && line_number >= normal_blocks.at(j).line_start))
                    {
                        counter = std::max(counter, normal_blocks.at(j).exec_count);
                    }
                }

                gooda::afdo_stack stack;
                stack.count = counter;
                stack.num_inst = 1; 

                gooda::afdo_pos position;
                position.func = function.name;
                position.file = function.file;
                position.line = line_number;
                position.discr = 0;

                stack.stack.push_back(std::move(position));

                function.stacks.push_back(std::move(stack));
            }
        }

        if(!inlined_block_sets.empty()){
            BOOST_ASSERT_MSG(report.has_src_file(i), "Something went wrong with BB collection");

            auto& asm_file = report.asm_file(i);

            //2. Handle inlined blocks
            for(auto& source_line : file){
                auto line_number = source_line.get_counter(file.column(LINE));

                if(line_number >= function.first_line && line_number <= function.last_line){
                    for(auto& block_set : inlined_block_sets){
                        auto& first_bb = block_set.at(0); //TODO assert size > 0

                        //There is always one inlned basic block set that match this point
                        if(first_bb.line_start == line_number){
                            //The caller position
                            //Note:: Do not move, will be used several times
                            gooda::afdo_pos caller_position;
                            caller_position.func = function.name;
                            caller_position.file = function.file;
                            caller_position.line = line_number;
                            caller_position.discr = 0;

                            for(auto& block : block_set){
                                for(auto j = block.gooda_line_start + 1; j < block.gooda_line_end; ++j){
                                    BOOST_ASSERT_MSG(j < asm_file.lines(), "Something went wrong with BB collection");

                                    auto& asm_line = asm_file.line(j);
                                    auto callee_line_number = asm_line.get_counter(asm_file.column(INIT_LINE));

                                    std::string callee_function_name = "asdf"; //TODO Get the function name
                
                                    data.add_file_name(callee_function_name);
                
                                    gooda::afdo_pos callee_position;
                                    callee_position.func = callee_function_name;
                                    callee_position.file = block.inlined_file;
                                    callee_position.line = callee_line_number;
                                    callee_position.discr = 0;
                                    
                                    data.add_file_name(block.inlined_file);
                
                                    gooda::afdo_stack stack;
                                    stack.count = block.exec_count;
                                    stack.num_inst = 2; 

                                    stack.stack.push_back(caller_position);
                                    stack.stack.push_back(std::move(callee_position));
                                    
                                    function.stacks.push_back(std::move(stack));
                                }
                            }

                            break;
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
    std::map<std::size_t, std::size_t> histogram;
    std::size_t total_count = 0;

    for(auto& function : data.functions){
        for(auto& stack : function.stacks){
            histogram[stack.count] += stack.num_inst;
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
        auto inst = rit->second;

        while(count * inst + accumulated_count > one_bucket_count * (bucket_num + 1)){
            auto offset = (one_bucket_count * (bucket_num + 1) - accumulated_count) / count;

            accumulated_inst += offset;
            accumulated_count += offset * count;

            inst -= offset;

            data.working_set[bucket_num].num_counter = accumulated_inst;
            data.working_set[bucket_num].min_counter = count;
            ++bucket_num;
        }

        accumulated_inst += inst;
        accumulated_count += inst * count;

        ++rit;
    }
}

std::string get_function_name(const std::string& application_file, std::vector<lbr_bb> block_set){
    log::emit<log::Debug>() << "Get function name with objdump" << log::endl;

    long start_address = std::numeric_limits<long>::max();
    for(auto& block : block_set){
        start_address = std::min(start_address, block.address);
    }

    long stop_address = start_address + 1;

    std::stringstream ss;
    ss << "objdump " << application_file << " -D --line-numbers --start-address=0x" << std::hex << start_address << " --stop-address=0x" << stop_address;

    std::string command = ss.str();
    auto result = gooda::exec_command_result(command);

    std::vector<std::string> lines;

    std::istringstream result_stream(result);
    std::string str_line;    
    while (std::getline(result_stream, str_line)) {
        lines.push_back(std::move(str_line));
    }

    auto function_name = lines.at(lines.size() - 3);
    function_name = function_name.substr(0, function_name.size() - 3);

    log::emit<log::Debug>() << "Found \"" << function_name << "\"" << log::endl;

    return function_name;
}

std::vector<std::vector<lbr_bb>> compute_inlined_sets(std::vector<std::vector<lbr_bb>> basic_block_sets){
    std::unordered_map<inlined_key, std::vector<lbr_bb>> inline_mappings;

    for(auto& block_set : basic_block_sets){
        for(auto& block : block_set){
            //If this block comes from an inlined function
            if(!block.inlined_file.empty()){
                inline_mappings[{block.file, block.line_start}].push_back(block);
            }
        }
    }
    
    std::vector<std::vector<lbr_bb>> inlined_sets;

    for(auto& pair : inline_mappings){
       inlined_sets.push_back(pair.second); 
    }

    return inlined_sets;
}

} //End of anonymous namespace

void gooda::read_report(const gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    bool lbr = vm.count("lbr");

    //TODO Find the file
    std::string application_file = "nbench";

    //Choose the correct counter
    std::string counter_name;
    if(lbr){
        counter_name = BB_EXEC;
    } else {
        counter_name = UNHALTED_CORE_CYCLES;
    }

    std::vector<std::vector<lbr_bb>> basic_block_sets;

    //First pass, only get basic information about the functions
    for(std::size_t i = 0; i < report.functions(); ++i){
        auto& line = report.hotspot_function(i);

        auto string_cycles = line.get_string(report.get_hotspot_file().column(counter_name));

        //Some functions are filled empty by Gooda for some reason
        //In some case, it means 0, in that case, it is not a problem to ignore it either, cause not really hotspot
        if(string_cycles.empty()){
            gooda::afdo_function function;
            function.valid = false;

            data.functions.push_back(function);

            continue;
        }
    
        gooda::afdo_function function;
        function.name = line.get_string(report.get_hotspot_file().column(FUNCTION_NAME));
        function.file = "unknown"; //The file will be filled by read_asm
        function.total_count = line.get_counter(report.get_hotspot_file().column(counter_name));

        data.add_file_name(function.file);
        data.add_file_name(function.name);

        data.functions.push_back(function);
        
        //Collect function.file and function.entry_count
        read_asm_file(report, i, data, counter_name);
    }
        
    //Second pass, get the inline stacks for all the functions
    //Necessary to make to pass for the functions to be present in the data structure
    //when inlined function are found in the source
    for(std::size_t i = 0; i < report.functions(); ++i){
        auto& function = data.functions.at(i);

        if(!function.valid){
            continue;
        }

        if(lbr){
            auto basic_blocks = collect_bb(report, i, counter_name);

            annotate_src_file(report, i, data, basic_blocks);
            
            basic_block_sets.push_back(std::move(basic_blocks));
        } else {
            read_src_file(report, i, data, counter_name);
        }
    }

    //Handle inlined functions in LBR mode
    if(lbr){
        auto inlined_block_sets = compute_inlined_sets(basic_block_sets);

        for(auto& block_set : inlined_block_sets){
            auto& first_block = block_set.at(0); //TODO test size > 0

            BOOST_ASSERT_MSG(!first_block.inlined_file.empty(), "All the blocks should be from inlined functions");

            bool found = false;
            
            for(auto& function : data.functions){
                if(function.file == first_block.inlined_file){
                    //TODO Find a better test for that
                    if(function.first_line == first_block.inlined_line_start){
                        for(auto& stack : function.stacks){
                            for(auto& pos : stack.stack){
                                auto line_number = pos.line;

                                for(std::size_t i = 0; i < block_set.size(); ++i){
                                    if(
                                            (i + 1 < block_set.size() && line_number >= block_set.at(i).inlined_line_start && line_number < block_set.at(i+1).inlined_line_start)
                                            ||  (i + 1 == block_set.size() && line_number >= block_set.at(i).inlined_line_start))
                                    {
                                        stack.count = std::max(stack.count, block_set.at(i).exec_count);
                                    }
                                }
                            }
                        }

                        found = true;
                        break;
                    }
                }
            }

            //It is possible that some inlined functions are not hotspot, for instance if all the calls to it
            //have been inlined
            //TODO Do something for that case
            if(!found){
                auto function_name = get_function_name(application_file, block_set);

                log::emit<log::Debug>() << "Create function " << function_name << " that was inlined" << log::endl;
                
                gooda::afdo_function function;
                function.name = function_name;
                function.file = first_block.inlined_file;
                function.entry_count = first_block.exec_count;

                function.total_count = 0;
                for(auto& block : block_set){
                    function.total_count = std::max(function.total_count, block.exec_count);
                }

                data.add_file_name(function.file);
                data.add_file_name(function.name);

                data.functions.push_back(function);

                BOOST_ASSERT_MSG(report.has_src_file(first_block.gooda_function), "Something went wrong with BB collection");

                //Get the source view of the caller function
                auto& file = report.src_file(first_block.gooda_function);

                /*for(auto& block : block_set){
                   for(auto i = block.gooda_line_start; i < block.gooda_line_end; ++i){
                       BOOST_ASSERT_MSG(i < file.lines(), "Something went wrong with BB collection");

                       auto& line = file.line(i);
                       auto line_number = line.get_counter(file.column(LINE));

                       if(line_number >= function.first_line && line_number <= function.last_line){
                           gcov_type counter = 0;

                           //Several basic blocks can be on the same line
                           //=> Take the max as the value of the line
                           for(std::size_t j = 0; j < block_set.size(); ++j){
                               if(
                                       (j + 1 < block_set.size() && line_number >= block_set[j].line_start && line_number < block_set[j + 1].line_start)
                                       ||  (j + 1 == block_set.size() && line_number >= block_set[j].line_start))
                               {
                                   counter = std::max(counter, block_set[j].exec_count);
                               }
                           }

                           gooda::afdo_stack stack;
                           stack.count = counter;
                           stack.num_inst = 1; 

                           gooda::afdo_pos position;
                           position.func = function.name;
                           position.file = function.file;
                           position.line = line_number;
                           position.discr = 0;

                           stack.stack.push_back(position);

                           function.stacks.push_back(std::move(stack));
                       }
                   }
                }*/
            }
        }
    }

    compute_working_set(data);
    compute_lengths(data);

    //Note: No need to fill the modules because it is not used by GCC
    //It will be automatically written empty by the AFDO generator
}
