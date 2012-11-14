#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <cstring>

#include "afdo_generator.hpp"

//Necessary for gcc_assert
//#include "tconfig.h"
//#include "tsystem.h"
//#include "coretypes.h"
//#include "tm.h"

#define IN_LIBGCOV 1
#define GCOV_LINKAGE // nothing
//#include "gcov-io.h"
//#include "gcov-io.c"

#include "gcov-iov.h"

#define GCOV_DATA_MAGIC ((gcov_unsigned_t)0x67636461) /* "gcda" */

//TODO Once GCC patched, these defines should be removed
#define GCOV_TAG_AFDO_FILE_NAMES ((gcov_unsigned_t)0xaa000000)
#define GCOV_TAG_AFDO_FUNCTION ((gcov_unsigned_t)0xac000000)
#define GCOV_TAG_AFDO_MODULE_GROUPING ((gcov_unsigned_t)0xae000000)
#define GCOV_TAG_AFDO_WORKING_SET ((gcov_unsigned_t)0xaf000000)

//Should be only temporary
std::ofstream gcov_file;

namespace {

void write_unsigned(gcov_unsigned_t value){
    //std::cout << value << std::endl;
    
    gcov_file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void write_counter(gcov_type value){
    //std::cout << value << std::endl;
    
    //gcov_file.write(reinterpret_cast<const char*>(&value), sizeof(value));

    gcov_unsigned_t lo = value >> 0;
    gcov_unsigned_t hi = value >> 32;

    write_unsigned(lo);
    write_unsigned(hi);
}

void gooda_gcov_write_string (const char *string){
    unsigned length = 0;
    unsigned alloc = 0;
    //gcov_unsigned_t *buffer;

    if (string)
    {   
        length = strlen (string);
        alloc = (length + 4) >> 2;
    }   

    //buffer = gcov_write_words (1 + alloc);
    
    gcov_file.write(reinterpret_cast<const char*>(&alloc), sizeof(alloc));

    char* buffer = new char[alloc * 4];

    for(unsigned int i = alloc - 1; i > length; --i){
        buffer[i] = 0;
    }

    //buffer[0] = alloc;
    //buffer[alloc - 1] = 0;
    memcpy (&buffer[0], string, length);
    
    gcov_file.write(buffer, alloc * 4);

    delete[] buffer;
}


/*static void write_int32(uint32_t i) {
      fwrite(&i, 4, 1, output_file);
}

static void write_int64(uint64_t i) {
      uint32_t lo = i >>  0;
        uint32_t hi = i >> 32;
          write_int32(lo);
            write_int32(hi);
}

static uint32_t length_of_string(const char *s) {
      return (strlen(s) / 4) + 1;
}

static void write_string(const char *s) {
      uint32_t len = length_of_string(s);
        write_int32(len);
          fwrite(s, strlen(s), 1, output_file);
            fwrite("\0\0\0\0", 4 - (strlen(s) % 4), 1, output_file);
}*/

void write_string (const std::string& value){
    //std::cout << value << std::endl;

    gooda_gcov_write_string(value.c_str());

    //assert(!gcov_is_error());
}

template<typename Type, typename Lambda>
void write_collection(const std::vector<Type>& values, Lambda functor){
    write_unsigned(values.size());

    std::for_each(values.begin(), values.end(), functor);
}

void write_header(){
    write_unsigned(GCOV_DATA_MAGIC);
    write_unsigned(GCOV_VERSION);
    //gcov_write_tag_length(GCOV_DATA_MAGIC, GCOV_VERSION);
    //assert(!gcov_is_error());
    
    //The stamp is not important for AFDO
    write_unsigned(0);
}

void write_section_header(gcov_unsigned_t tag){
    //The header of the section
    write_unsigned(tag);

    //The size of the section, skipped by AFDO
    write_unsigned(0);
}

void write_file_name_table(const gooda::afdo_data& data){
    write_section_header(GCOV_TAG_AFDO_FILE_NAMES);

    write_collection(data.file_names, write_string);
}

void write_function_table(const gooda::afdo_data& data){
    write_section_header(GCOV_TAG_AFDO_FUNCTION);

    write_collection(data.functions, [&data](const gooda::afdo_function& function){
        write_string(function.name);

        write_unsigned(data.get_file_index(function.file));

        write_counter(function.total_count);
        write_counter(function.entry_count);

        write_collection(function.stacks, [&data](const gooda::afdo_stack& stack){
            write_collection(stack.stack, [&data](const gooda::afdo_pos& s){
                write_unsigned(data.get_file_index(s.func));
                write_unsigned(data.get_file_index(s.file));
                
                write_unsigned(s.line);
                write_unsigned(s.discr);
            });
            
            write_counter(stack.count);
            write_counter(stack.num_inst);
        });
    });
}

void write_module_info(const gooda::afdo_data& data){
    write_section_header(GCOV_TAG_AFDO_MODULE_GROUPING);

    write_collection(data.modules, [&data](const gooda::afdo_module& module){
        write_string(module.name);

        write_unsigned(module.exported);
        write_unsigned(module.has_asm);
        
        write_unsigned(module.num_aux_modules);
        write_unsigned(module.num_quote_paths);
        write_unsigned(module.num_bracket_paths);
        write_unsigned(module.num_cpp_defines);
        write_unsigned(module.num_cpp_includes);
        write_unsigned(module.num_cl_args);

        std::for_each(module.strings.begin(), module.strings.end(), write_string);
    });
}

void write_working_set(const gooda::afdo_data& data){
    write_section_header(GCOV_TAG_AFDO_WORKING_SET);

    std::for_each(data.working_set.begin(), data.working_set.end(), [](const gooda::afdo_working_set& ws){
        write_unsigned(ws.num_counter);
        write_counter(ws.min_counter);
    });
}

} //end of anonymous namespace

void gooda::generate_afdo(const afdo_data& data, const std::string& file){
    std::cout << "Generate AFDO profile in \"" << file << "\"" << std::endl;

    gcov_file.open(file.c_str(), std::ios::binary | std::ios::out );

    /*if(!gcov_open(file.c_str())){
        std::cout << "Cannot open file for writing" << std::endl;
        return;
    }*/

    if(!gcov_file){
        std::cout << "Cannot open file for writing" << std::endl;
        return;
    }

    //Put GCOV in write mode
    //gcov_rewrite();
    //assert(!gcov_is_error());

    write_header();

    write_file_name_table(data);
    write_function_table(data);
    write_module_info(data);
    write_working_set(data);

    //write_unsigned(0);
    //gcov_close();
}
