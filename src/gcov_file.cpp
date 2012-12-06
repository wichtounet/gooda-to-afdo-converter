//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cstring>

#include "gcov_file.hpp"

bool gooda::gcov_file::open(const std::string& file){
    return open_for_write(file);
}

bool gooda::gcov_file::open_for_write(const std::string& file){
    gcov_file_w.open(file.c_str(), std::ios::binary | std::ios::out );

    return gcov_file_w;
}

bool gooda::gcov_file::open_for_read(const std::string& file){
    gcov_file_r.open(file.c_str(), std::ios::binary | std::ios::in );

    return gcov_file_r;
}

//Writing

void gooda::gcov_file::write_unsigned(gcov_unsigned_t value){
    gcov_file_w.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void gooda::gcov_file::write_counter(gcov_type value){
    write_unsigned(value >> 0);
    write_unsigned(value >> 32);
}

void gooda::gcov_file::write_string (const std::string& value){
    const char* string = value.c_str();

    unsigned int length = strlen (string);
    unsigned int alloc = (length + 4) >> 2;
    
    write_unsigned(alloc);

    char* buffer = new char[alloc * 4];
    std::fill(buffer, buffer+(alloc*4), 0);

    memcpy (&buffer[0], string, length);
    
    gcov_file_w.write(buffer, alloc * 4);

    delete[] buffer;
}

void gooda::gcov_file::write_header(){
    write_unsigned(GCOV_DATA_MAGIC);
    write_unsigned(GCOV_VERSION);
    
    //The stamp is not important for AFDO
    write_unsigned(0);
}

void gooda::gcov_file::write_section_header(gcov_unsigned_t tag, unsigned int length){
    //The header of the section
    write_unsigned(tag);

    //The size of the section, skipped by AFDO, but important to make a GCOV-valid file
    write_unsigned(length);
}

//Reading

gcov_unsigned_t gooda::gcov_file::read_unsigned(){
    gcov_unsigned_t value;
    gcov_file_r.read(reinterpret_cast<char*>(&value), sizeof(gcov_unsigned_t));
    return value;
}
