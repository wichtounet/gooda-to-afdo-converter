#include <iostream>

#include "generator.hpp"

//Necessary for gcc_assert
#include "tsystem.h"

#define IN_LIBGCOV 1
#define GCOV_LINKAGE /* nothing */
#include "gcov-io.h"
#include "gcov-io.c"

void write_header();

void converter::generate_afdo(Data& data, const std::string& file){
    std::cout << "Generate AFDO profile in \"" << file << "\"" << std::endl;

    if(!gcov_open(file.c_str())){
        std::cout << "Cannot open file for writing" << std::endl;
        return;
    }

    gcov_rewrite();

    write_header();

    gcov_close();
}

void write_header(){
    gcov_write_unsigned(GCOV_DATA_MAGIC);
    gcov_write_unsigned(GCOV_VERSION);
    gcov_write_unsigned(0); //The stamp is not important for AFDO
}
