#include <iostream>
#include <chrono>

#include "utils.hpp"
#include "gooda_reader.hpp"
#include "converter.hpp"
#include "afdo_generator.hpp"

namespace {

//Chrono typedefs
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds milliseconds;

void print_usage(){
    std::cout << "Usage: converter spreadsheets_directory" << std::endl;
}

void process(const std::string& directory){
    Clock::time_point t0 = Clock::now();

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets(directory);

    gooda::afdo_data data;

    //Read the report and generate AFDO file
    gooda::read_report(report, data);
    gooda::generate_afdo(data, "generated.afdo");
    
    Clock::time_point t1 = Clock::now();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - t0);

    std::cout << "Conversion took " << ms.count() << "ms" << std::endl;
}

} //end of anonymous namespace

int main(int argc, char **argv){
    if(argc == 1){
        std::cout << "Not enough arguments" << std::endl;
        print_usage();

        return 1;
    }

    std::string directory(argv[1]);

    if(!gooda::exists(directory)){
        std::cout << "\"" << directory << "\" does not exists" << std::endl;
        return 1;
    }
    
    if(!gooda::is_directory(directory)){
        std::cout << "\"" << directory << "\" is not a directory" << std::endl;
        return 1;
    }

    process(directory);

    return 0;
}
