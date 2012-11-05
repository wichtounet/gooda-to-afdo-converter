#include <iostream>
#include <algorithm>

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

template<typename Type, typename Lambda>
void write_collection(const std::vector<Type>& values, Lambda functor){
    gcov_write_unsigned(values.size());

    std::for_each(values.begin(), values.end(), functor);
}

void write_section_header(gcov_unsigned_t tag){
    //The header of the section
    gcov_write_unsigned(tag);
    gcov_write_unsigned(0); //Skipped by AFDO
}

void write_header(){
    gcov_write_unsigned(GCOV_DATA_MAGIC);
    gcov_write_unsigned(GCOV_VERSION);
    gcov_write_unsigned(0); //The stamp is not important for AFDO
}

void write_file_name_table(converter::Data& data){
    write_section_header(GCOV_TAG_AFDO_FILE_NAMES);

    write_collection(data.file_names, write_string);
}

void write_function_table(converter::Data& data){
    write_section_header(GCOV_TAG_AFDO_FUNCTION);

    write_collection(data.functions, [&data](const converter::Function& function){
        write_string(function.name);

        gcov_write_unsigned(data.get_file_index(function.file));

        gcov_write_counter(function.total_count);
        gcov_write_counter(function.entry_count);

        write_collection(function.stacks, [&data](const converter::Stack& stack){
            write_collection(stack.stack, [&data](const converter::CallSitePos& s){
                gcov_write_unsigned(data.get_file_index(s.func));
                gcov_write_unsigned(data.get_file_index(s.file));
                
                gcov_write_unsigned(s.line);
                gcov_write_unsigned(s.discr);
            });
            
            gcov_write_counter(stack.count);
            gcov_write_counter(stack.num_inst);
        });
    });
}

void write_module_info(converter::Data& data){
    write_section_header(GCOV_TAG_AFDO_MODULE_GROUPING);

    write_collection(data.modules, [&data](const converter::Module& module){
        write_string(module.name);

        gcov_write_unsigned(module.exported);
        gcov_write_unsigned(module.has_asm);
        
        gcov_write_unsigned(module.num_aux_modules);
        gcov_write_unsigned(module.num_quote_paths);
        gcov_write_unsigned(module.num_bracket_paths);
        gcov_write_unsigned(module.num_cpp_defines);
        gcov_write_unsigned(module.num_cpp_includes);
        gcov_write_unsigned(module.num_cl_args);

        std::for_each(module.strings.begin(), module.strings.end(), write_string);
    });
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
