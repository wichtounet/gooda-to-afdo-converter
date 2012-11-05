#include <iostream>

#include "generator.hpp"

//Necessary for gcc_assert
#include "tsystem.h"

#define IN_LIBGCOV 1
#define GCOV_LINKAGE /* nothing */
#include "gcov-io.h"
#include "gcov-io.c"

//TODO Once GCC patched, these defines should be removed
#define GCOV_TAG_AFDO_FILE_NAMES ((gcov_unsigned_t)0xaa000000)
#define GCOV_TAG_AFDO_FUNCTION ((gcov_unsigned_t)0xac000000)
#define GCOV_TAG_AFDO_MODULE_GROUPING ((gcov_unsigned_t)0xae000000)
#define GCOV_TAG_AFDO_WORKING_SET ((gcov_unsigned_t)0xaf000000)

namespace {

void write_string (const std::string& value){
    unsigned length = value.size();
    unsigned alloc = (length + 4) >> 2;
    
    gcov_unsigned_t *buffer;
    buffer = gcov_write_words (1 + alloc);

    buffer[0] = alloc;
    buffer[alloc] = 0;
    memcpy (&buffer[1], value.c_str(), length);
}

void write_header(){
    gcov_write_unsigned(GCOV_DATA_MAGIC);
    gcov_write_unsigned(GCOV_VERSION);
    gcov_write_unsigned(0); //The stamp is not important for AFDO
}

void write_section_header(gcov_unsigned_t tag){
    //The header of the section
    gcov_write_unsigned(tag);
    gcov_write_unsigned(0); //Skipped by AFDO
}

void write_file_name_table(converter::Data& data){
    write_section_header(GCOV_TAG_AFDO_FILE_NAMES);

    //The number of files
    gcov_write_unsigned(data.file_names.size());

    for(auto& file : data.file_names){
        write_string(file);
    }
}

void write_function_table(converter::Data& data){
    write_section_header(GCOV_TAG_AFDO_FUNCTION);
    
    //The number of functions 
    gcov_write_unsigned(data.functions.size());

    for(auto& function : data.functions){
        write_string(function.name);
        gcov_write_unsigned(data.get_file_index(function.file));
        gcov_write_counter(function.total_count);
        gcov_write_counter(function.entry_count);

        //The number of stacks
        gcov_write_counter(function.stacks.size());

        for(auto& stack : function.stacks){
            //The number of stacks
            gcov_write_unsigned(stack.stack.size());

            for(auto& s : stack.stack){
                gcov_write_unsigned(data.get_file_index(s.func));
                gcov_write_unsigned(data.get_file_index(s.file));
                gcov_write_unsigned(s.line);
                gcov_write_unsigned(s.discr);
            }

            gcov_write_counter(stack.count);
            gcov_write_counter(stack.num_inst);
        }
    }
}

void write_module_info(converter::Data& data){
    write_section_header(GCOV_TAG_AFDO_MODULE_GROUPING);
    
    //The number of modules
    gcov_write_unsigned(data.modules.size());

    for(auto& module : data.modules){
        write_string(module.name);
        gcov_write_unsigned(module.exported);
        gcov_write_unsigned(module.has_asm);
        gcov_write_unsigned(module.num_aux_modules);
        gcov_write_unsigned(module.num_quote_paths);
        gcov_write_unsigned(module.num_bracket_paths);
        gcov_write_unsigned(module.num_cpp_defines);
        gcov_write_unsigned(module.num_cpp_includes);
        gcov_write_unsigned(module.num_cl_args);

        for(auto& string : module.strings){
            write_string(string);
        }
    }
}

} //end of anonymous namespace

void converter::generate_afdo(Data& data, const std::string& file){
    std::cout << "Generate AFDO profile in \"" << file << "\"" << std::endl;

    if(!gcov_open(file.c_str())){
        std::cout << "Cannot open file for writing" << std::endl;
        return;
    }

    //Put GCOV in write mode
    gcov_rewrite();

    write_header();
    write_file_name_table(data);
    write_function_table(data);
    write_module_info(data);

    gcov_close();
}
