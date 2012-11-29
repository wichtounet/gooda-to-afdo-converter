#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <map>

#include <boost/algorithm/string.hpp>

#include "converter.hpp"
#include "utils.hpp"

namespace {

//Normal mode

void read_asm_file(const gooda::gooda_report& report, std::size_t i, gooda::afdo_data& data, const std::string& counter){
    if(report.has_asm_file(i)){
        auto& function = data.functions[i];
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
            auto disassembly = line.get_string(file.column(DISASSEMBLY));
            
            //Get the entry basic block
            if(boost::starts_with(disassembly, "Basic Block 1 <")){
                function.entry_count = line.get_counter(file.column(counter));

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

void read_src_file(const gooda::gooda_report& report, std::size_t i, gooda::afdo_data& data, const std::string& counter){
    if(report.has_src_file(i)){
        auto& function = data.functions[i];

        auto& file = report.src_file(i);

        for(auto& line : file){
            auto line_number = line.get_counter(file.column(LINE));

            if(line_number >= function.first_line && line_number <= function.last_line){
                gooda::afdo_stack stack;
                stack.count = line.get_counter(file.column(counter));
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
    unsigned long line_start;
    unsigned long exec_count;
    
    unsigned long inlined_line_start;
    std::string inlined_file;
};

std::vector<lbr_bb> collect_bb(const gooda::gooda_report& report, std::size_t i, const std::string& counter){
    std::vector<lbr_bb> basic_blocks;

    if(report.has_asm_file(i)){
        auto& file = report.asm_file(i);

        for(std::size_t i = 0; i < file.lines(); ++i){
            auto& line = file.line(i);
            
            auto disassembly = line.get_string(file.column(DISASSEMBLY));
            
            if(boost::starts_with(disassembly, "Basic Block ")){
                lbr_bb block;
                
                block.line_start = line.get_counter(file.column(PRINC_LINE));
                block.exec_count = line.get_counter(file.column(counter));

                //By default considered as not coming from inlined function
                block.inlined_line_start = 0;

                if(i + 1 < file.lines()){
                    auto& next_line = file.line(i + 1);

                    //If the next line is part of the same basic block
                    if(next_line.get_counter(file.column(PRINC_LINE)) == block.line_start){
                        auto princ_file = next_line.get_string(file.column(PRINC_FILE));
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

void annotate_src_file(const gooda::gooda_report& report, std::size_t i, gooda::afdo_data& data, std::vector<lbr_bb>& basic_blocks){
    if(report.has_src_file(i)){
        auto& function = data.functions[i];

        auto& file = report.src_file(i);

        for(auto& line : file){
            auto line_number = line.get_counter(file.column(LINE));

            if(line_number >= function.first_line && line_number <= function.last_line){
                gcov_type counter = 0;

                //Several basic blocks can be on the same line
                //=> Take the max as the value of the line
                for(std::size_t i = 0; i < basic_blocks.size(); ++i){
                    if(
                            (i + 1 < basic_blocks.size() && line_number >= basic_blocks[i].line_start && line_number < basic_blocks[i+1].line_start)
                            ||  (i + 1 == basic_blocks.size() && line_number >= basic_blocks[i].line_start))
                    {
                        if(basic_blocks[i].exec_count > counter){
                            counter = basic_blocks[i].exec_count;
                        }
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

} //End of anonymous namespace

void gooda::read_report(const gooda_report& report, gooda::afdo_data& data, boost::program_options::variables_map& vm){
    bool lbr = vm.count("lbr");

    //Choose the correct counter
    std::string counter;
    if(lbr){
        counter = BB_EXEC;
    } else {
        counter = UNHALTED_CORE_CYCLES;
    }

    std::vector<std::vector<lbr_bb>> basic_block_sets;

    for(std::size_t i = 0; i < report.functions(); ++i){
        auto& line = report.hotspot_function(i);

        auto string_cycles = line.get_string(report.get_hotspot_file().column(counter));

        //Some functions are filled empty by Gooda for some reason
        //In some case, it means 0, in that case, it is not a problem to ignore it either, cause not really hotspot
        if(string_cycles.empty()){
            continue;
        }
    
        gooda::afdo_function function;
        function.name = line.get_string(report.get_hotspot_file().column(FUNCTION_NAME));
        function.file = "unknown"; //The file will be filled by read_asm
        function.total_count = line.get_counter(report.get_hotspot_file().column(counter));

        data.add_file_name(function.file);
        data.add_file_name(function.name);

        data.functions.push_back(function);
        
        //Collect function.file and function.entry_count
        read_asm_file(report, i, data, counter);
        
        if(lbr){
            auto basic_blocks = collect_bb(report, i, counter);

            //TODO Some aggregation of the results (inlining)
            
            annotate_src_file(report, i, data, basic_blocks);
            
            basic_block_sets.push_back(std::move(basic_blocks));
        } else {
            read_src_file(report, i, data, counter);
        }
    }

    //Handle inlined functions in LBR mode
    if(lbr){
        for(auto& block_set : basic_block_sets){
            for(std::size_t i = 0; i < block_set.size(); ++i){
                auto& block = block_set[i];
                if(!block.inlined_file.empty()){
                    for(auto& function : data.functions){
                        if(function.file == block.inlined_file){
                            for(auto& stack : function.stacks){
                                for(auto& pos : stack.stack){
                                    if(pos.line == block.inlined_line_start){
                                        std::cout << function.name << " has been inlined" << std::endl;

                                        auto princ_line = block.line_start;
                                        std::cout << princ_line << std::endl;
                                        std::cout << block.inlined_line_start << std::endl;

                                        for(; i < block_set.size() && block_set[i].line_start == princ_line; ++i){
                                            std::cout << "Found source BB" << std::endl;
                                        }

                                        --i;

                                        //TODO Iterate through all the basic blocks that have the same source line (in the caller)
                                        //And do the same operations that was done in annotate_src_file
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    compute_working_set(data);

    compute_lengths(data);

    //Note: No need to fill the modules because it is not used by GCC
    //It will be automatically written empty by the AFDO generator
}
