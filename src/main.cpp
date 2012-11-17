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
            ("profile,p", "Profile the given application")
            ("input-file", po::value<std::vector<std::string>>()->required(), "Directory containing the spreadsheets");

        po::positional_options_description p;
        p.add("input-file", -1);

        po::variables_map vm;
        
        auto parsed = po::command_line_parser(argc, argv).options(description).positional(p).allow_unregistered().run();
        po::store(parsed, vm);

        if(vm.count("help")){
            std::cout << description;
            return 0;
        }

        //Profiling mode
        if(vm.count("profile")){
            auto further_options = po::collect_unrecognized(parsed.options, po::include_positional);

            std::string command;

            for(auto& option : further_options){
                command += option + " ";
            }

            std::cout << "Profile the given application" << std::endl;
            auto output = gooda::exec_command(command);
            std::cout << output << std::endl;

            return 0;
        }
        
        //Must be done after the test for help/profile to handle required arguments
        po::notify(vm);
       
        //No further options are allowed if not in profile mode
        auto further_options = po::collect_unrecognized(parsed.options, po::exclude_positional);
        if(!further_options.empty()){
            std::cerr << "Error: Unrecognized options " << further_options[0];

            for(std::size_t i = 1; i < further_options.size(); ++i){
                std::cerr << ", " << further_options[i];
            }

            std::cerr << std::endl;

            return 1;
        }

        auto input_files = vm["input-file"].as<std::vector<std::string>>();
        
        //Verify that only one directory is provided
        if(input_files.size() > 1){
            std::cerr << "Error: Only one directory can be analyzed at a time" << std::endl;

            return 1;
        }

        std::string directory = input_files[0];

        //The file must exists
        if(!gooda::exists(directory)){
            std::cerr << "Error \"" << directory << "\" does not exists" << std::endl;
            return 1;
        }

        //The file must be a directory
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
