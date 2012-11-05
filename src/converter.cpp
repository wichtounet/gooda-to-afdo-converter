#include <iostream>

#include "utils.hpp"
#include "reader.hpp"
#include "generator.hpp"

void print_usage();

int main(int argc, char **argv){
    if(argc == 1){
        std::cout << "Not enough arguments" << std::endl;
        print_usage();

        return 1;
    }

    std::string directory(argv[1]);

    if(!converter::exists(directory)){
        std::cout << "\"" << directory << "\" does not exists" << std::endl;
        return 1;
    }
    
    if(!converter::is_directory(directory)){
        std::cout << "\"" << directory << "\" is not a directory" << std::endl;
        return 1;
    }

    converter::Data data;

    if(converter::read_spreadsheets(directory, data)){
        converter::generate_afdo(data, "generated.afdo");
    }

    return 0;
}

void print_usage(){
    std::cout << "Usage: converter spreadsheets_directory" << std::endl;
}
