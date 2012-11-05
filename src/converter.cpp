#include <iostream>

void print_usage();

int main(int argc, char **argv){
    if(argc == 1){
        std::cout << "Not enough arguments" << std::endl;
        print_usage();

        return 1;
    }

    std::string directory(argv[0]);
    
    std::cout << directory << std::endl;

    return 0;
}

void print_usage(){
    std::cout << "Usage: converter spreadsheets_directory" << std::endl;
}
