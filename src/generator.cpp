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

void write_file_name_table(converter::Data& data){
    //The header of the section
    gcov_write_unsigned(GCOV_TAG_AFDO_FILE_NAMES);
    gcov_write_unsigned(0); //Skipped by AFDO

    //The number of files
    gcov_write_unsigned(data.file_names.size());

    for(auto& file : data.file_names){
        write_string(file);
    }
}

} //end of anonymous namespace

void converter::generate_afdo(Data& data, const std::string& file){
    std::cout << "Generate AFDO profile in \"" << file << "\"" << std::endl;

    if(!gcov_open(file.c_str())){
        std::cout << "Cannot open file for writing" << std::endl;
        return;
    }

    gcov_rewrite();

    write_header();
    write_file_name_table(data);

    gcov_close();
}
