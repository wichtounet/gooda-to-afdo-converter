#include <algorithm>

#include "afdo_generator.hpp"
#include "gcov_file.hpp"

namespace {

template<typename Type, typename Lambda>
void write_collection(const std::vector<Type>& values, gooda::gcov_file& gcov_file, Lambda functor){
    gcov_file.write_unsigned(values.size());

    std::for_each(values.begin(), values.end(), functor);
}

void write_file_name_table(const gooda::afdo_data& data, gooda::gcov_file& gcov_file){
    gcov_file.write_section_header(GCOV_TAG_AFDO_FILE_NAMES, data.length_file_section);

    gcov_file.write_unsigned(data.file_names.size());

    for(auto& file_name : data.file_names){
        gcov_file.write_string(file_name);
    }
}

void write_function_table(const gooda::afdo_data& data, boost::program_options::variables_map& vm, gooda::gcov_file& gcov_file){
    gcov_file.write_section_header(GCOV_TAG_AFDO_FUNCTION, data.length_function_section);

    write_collection(data.functions, gcov_file, [&data,&vm, &gcov_file](const gooda::afdo_function& function){
        gcov_file.write_string(function.name);

        gcov_file.write_unsigned(data.get_file_index(function.file));

        gcov_file.write_counter(function.total_count);
        gcov_file.write_counter(function.entry_count);

        write_collection(function.stacks, gcov_file, [&data,&vm, &gcov_file](const gooda::afdo_stack& stack){
            write_collection(stack.stack, gcov_file, [&data, &gcov_file](const gooda::afdo_pos& s){
                gcov_file.write_unsigned(data.get_file_index(s.func));
                gcov_file.write_unsigned(data.get_file_index(s.file));
                
                gcov_file.write_unsigned(s.line);
                gcov_file.write_unsigned(s.discr);
            });
            
            gcov_file.write_counter(stack.count);
            gcov_file.write_counter(stack.num_inst);

            if(vm.count("cache-misses")){
                gcov_file.write_counter(stack.cache_misses);
            }
        });
    });
}

void write_module_info(const gooda::afdo_data& data, gooda::gcov_file& gcov_file){
    gcov_file.write_section_header(GCOV_TAG_AFDO_MODULE_GROUPING, data.length_modules_section);

    write_collection(data.modules, gcov_file, [&data, &gcov_file](const gooda::afdo_module& module){
        gcov_file.write_string(module.name);

        gcov_file.write_unsigned(module.exported);
        gcov_file.write_unsigned(module.has_asm);
        
        gcov_file.write_unsigned(module.num_aux_modules);
        gcov_file.write_unsigned(module.num_quote_paths);
        gcov_file.write_unsigned(module.num_bracket_paths);
        gcov_file.write_unsigned(module.num_cpp_defines);
        gcov_file.write_unsigned(module.num_cpp_includes);
        gcov_file.write_unsigned(module.num_cl_args);

        for(auto& mod : module.strings){
            gcov_file.write_string(mod);
        }
    });
}

void write_working_set(const gooda::afdo_data& data, gooda::gcov_file& gcov_file){
    gcov_file.write_section_header(GCOV_TAG_AFDO_WORKING_SET, data.length_working_set_section);

    std::for_each(data.working_set.begin(), data.working_set.end(), [&gcov_file](const gooda::afdo_working_set& ws){
        gcov_file.write_unsigned(ws.num_counter);
        gcov_file.write_counter(ws.min_counter);
    });
}

} //end of anonymous namespace

void gooda::generate_afdo(const afdo_data& data, const std::string& file, boost::program_options::variables_map& vm){
    std::cout << "Generate AFDO profile in \"" << file << "\"" << std::endl;

    gooda::gcov_file gcov_file;

    if(!gcov_file.open(file)){
        std::cout << "Cannot open file for writing" << std::endl;
        return;
    }

    gcov_file.write_header();

    write_file_name_table(data, gcov_file);
    write_function_table(data, vm, gcov_file);
    write_module_info(data, gcov_file);
    write_working_set(data, gcov_file);
}
