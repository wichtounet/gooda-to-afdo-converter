#include <iostream>
#include <chrono>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "utils.hpp"
#include "gooda_reader.hpp"
#include "converter.hpp"
#include "afdo_generator.hpp"
#include "afdo_printer.hpp"
    
namespace po = boost::program_options;

namespace {

//Chrono typedefs
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds milliseconds;

void process(const std::string& directory, po::variables_map& vm){
    Clock::time_point t0 = Clock::now();

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets(directory);

    gooda::afdo_data data;

    //Read the report and generate AFDO file
    gooda::read_report(report, data);

    if(vm.count("dump")){
        gooda::dump_afdo(data);
    } else {
        gooda::generate_afdo(data, vm["output"].as<std::string>());
    }
    
    Clock::time_point t1 = Clock::now();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - t0);

    std::cout << "Conversion took " << ms.count() << "ms" << std::endl;
}

} //end of anonymous namespace

int main(int argc, char **argv){
    try {
        po::options_description description("converter [options] spreadsheets_directory");

        description.add_options()
            ("help,h", "Display this help message")
            ("output,o", po::value<std::string>()->default_value("fbdata.afdo"), "The name of the generated AFDO file")
            ("dump", "Dump the AFDO on standard output")
            ("input-file", po::value<std::string>()->required(), "Directory containing the spreadsheets");

        po::positional_options_description p;
        p.add("input-file", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(description).positional(p).run(), vm);

        if(vm.count("help")){
            std::cout << description;
            return 0;
        }
        
        //Must be done after the test for help to handle required arguments
        po::notify(vm);

        std::string directory = vm["input-file"].as<std::string>();

        if(!gooda::exists(directory)){
            std::cerr << "Error \"" << directory << "\" does not exists" << std::endl;
            return 1;
        }

        if(!gooda::is_directory(directory)){
            std::cerr << "Error: \"" << directory << "\" is not a directory" << std::endl;
            return 1;
        }

        process(directory, vm);
    } catch (std::exception& e ) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
