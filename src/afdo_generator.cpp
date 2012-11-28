#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <cstring>

#include "afdo_generator.hpp"

#define GCOV_VERSION ((gcov_unsigned_t)0x3430372a)      /* 407* */
#define GCOV_DATA_MAGIC ((gcov_unsigned_t)0x67636461)   /* "gcda" */

//AFDO tag names
#define GCOV_TAG_AFDO_FILE_NAMES ((gcov_unsigned_t)0xaa000000)
#define GCOV_TAG_AFDO_FUNCTION ((gcov_unsigned_t)0xac000000)
#define GCOV_TAG_AFDO_MODULE_GROUPING ((gcov_unsigned_t)0xae000000)
#define GCOV_TAG_AFDO_WORKING_SET ((gcov_unsigned_t)0xaf000000)

//TODO Do not let this as a global variable
std::ofstream gcov_file;

namespace {

void write_unsigned(gcov_unsigned_t value){
    gcov_file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void write_counter(gcov_type value){
    write_unsigned(value >> 0);
    write_unsigned(value >> 32);
}

void write_string (const std::string& value){
    const char* string = value.c_str();
    unsigned int length = 0;

    unsigned int alloc = 0;

    if (string)
    {   
        length = strlen (string);
        alloc = (length + 4) >> 2;
    }   
    
    write_unsigned(alloc);

    char* buffer = new char[alloc * 4];

    for(unsigned int i = 0; i < alloc * 4; ++i){
        buffer[i] = 0;
    }

    memcpy (&buffer[0], string, length);
    
    gcov_file.write(buffer, alloc * 4);

    delete[] buffer;
}

template<typename Type, typename Lambda>
void write_collection(const std::vector<Type>& values, Lambda functor){
    write_unsigned(values.size());

    std::for_each(values.begin(), values.end(), functor);
}

void write_header(){
    write_unsigned(GCOV_DATA_MAGIC);
    write_unsigned(GCOV_VERSION);
    
    //The stamp is not important for AFDO
    write_unsigned(0);
}

void write_section_header(gcov_unsigned_t tag, unsigned int length){
    //The header of the section
    write_unsigned(tag);

    //The size of the section, skipped by AFDO, but important to make a GCOV-valid file
    write_unsigned(length);
}

void write_file_name_table(const gooda::afdo_data& data){
    write_section_header(GCOV_TAG_AFDO_FILE_NAMES, data.length_file_section);

    write_collection(data.file_names, write_string);
}

void write_function_table(const gooda::afdo_data& data, boost::program_options::variables_map& vm){
    write_section_header(GCOV_TAG_AFDO_FUNCTION, data.length_function_section);

    write_collection(data.functions, [&data,&vm](const gooda::afdo_function& function){
        write_string(function.name);

        write_unsigned(data.get_file_index(function.file));

        write_counter(function.total_count);
        write_counter(function.entry_count);

        write_collection(function.stacks, [&data,&vm](const gooda::afdo_stack& stack){
            write_collection(stack.stack, [&data](const gooda::afdo_pos& s){
                write_unsigned(data.get_file_index(s.func));
                write_unsigned(data.get_file_index(s.file));
                
                write_unsigned(s.line);
                write_unsigned(s.discr);
            });
            
            write_counter(stack.count);
            write_counter(stack.num_inst);

            if(vm.count("cache-misses")){
                write_counter(stack.cache_misses);
            }
        });
    });
}

void write_module_info(const gooda::afdo_data& data){
    write_section_header(GCOV_TAG_AFDO_MODULE_GROUPING, data.length_modules_section);

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
    write_section_header(GCOV_TAG_AFDO_WORKING_SET, data.length_working_set_section);

    std::for_each(data.working_set.begin(), data.working_set.end(), [](const gooda::afdo_working_set& ws){
        write_unsigned(ws.num_counter);
        write_counter(ws.min_counter);
    });
}

} //end of anonymous namespace

void gooda::generate_afdo(const afdo_data& data, const std::string& file, boost::program_options::variables_map& vm){
    std::cout << "Generate AFDO profile in \"" << file << "\"" << std::endl;

    gcov_file.open(file.c_str(), std::ios::binary | std::ios::out );

    if(!gcov_file){
        std::cout << "Cannot open file for writing" << std::endl;
        return;
    }

    write_header();

    write_file_name_table(data);
    write_function_table(data, vm);
    write_module_info(data);
    write_working_set(data);
}
