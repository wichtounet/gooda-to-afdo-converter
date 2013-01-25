//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "Options.hpp"
#include "logger.hpp"

int gooda::options::parse(int argc, const char **argv){
    try {
        po::options_description input("Input actions");
        input.add_options()
            ("read-spreadsheets", "Read Gooda spreadsheets (default)")
            ("read-afdo", "Read an existing AFDO profile")
            ("profile,p", "Profile the given application.")
            ("diff", "Diff between two sets of spreadsheets (prototype)")
            ;
        
        po::options_description output("Output actions");
        output.add_options()
            ("dump", "Dump AFDO profile on standard output")
            ("full-dump", "Dump complete AFDO profile on standard output")
            ("afdo", "Generate an AFDO profile (default if --profile is not selected)")
            ;
        
        po::options_description afdo("AFDO Options");
        afdo.add_options()
            ("lbr", "Performs precise profile with LBR (The default is cycle accounting)")
            ("auto", "Detect the type of the spreadsheets (Not valid with profile)")
            ("nows", "Do not compute the working set")
            ("cache-misses", "Fill cache misses information in the AFDO file")
            ("discriminators", "Find the DWARF discriminators of instructions, need >=binutils.2.23.1")
            ;

        po::options_description others("Other Options");
        others.add_options()
            ("help,h", "Display this help message")
            
            ("bench", "Use the special scripts for benchmarking")
            ("debug", "Make the dump more verbose")

            //Ideally filter would have a std::string with an implicit empty string
            //There is a bug in Boost PO that prevent implicit value and positional options at the same time
            ("filter,f", "Only consider the hottest process.")
            ("process", po::value<std::string>(), "Filter the hotspot functions by process.")
            ("output,o", po::value<std::string>()->default_value("fbdata.afdo"), "The name of the generated AFDO file")
            ("log", po::value<int>()->default_value(0), "Define the logging verbosity (0: No logging, 1: warnings, 2:debug 3:trace)")
            ("quiet", "Output as less as possible on the console")

            ("gooda", po::value<std::string>(), "Path to the Gooda installation directory. By default, $GOODA_DIR or the current directory will be used")
            ("addr2line", po::value<std::string>()->default_value("addr2line"), "Specify the addr2line executable to use")
            ("folder", po::value<std::string>()->default_value(""), "Specify in which to search the executable")
            ("input-file", po::value<std::vector<std::string>>(), "Input file(s)");

        description.add(input).add(output).add(afdo).add(others);

        po::positional_options_description p;
        p.add("input-file", -1);

        parsed_options = po::command_line_parser(argc, argv).options(description).positional(p).allow_unregistered().run();
        po::store(parsed_options, vm);

        if(vm.count("help")){
            std::cout << description;
            return 1;
        }

        if(vm.count("profile") && vm.count("auto")){
            log::emit<log::Error>() << "--auto and --profile cannot be used together" << log::endl;
            return 2;
        }
        
        if(vm.count("lbr") && vm.count("auto")){
            log::emit<log::Error>() << "--auto and --lbr cannot be used together" << log::endl;
            return 2;
        }
    } catch (std::exception& e ) {
        log::emit<log::Error>() << e.what() << log::endl;
        return 2;
    }
    
    log::set_level(vm["log"].as<int>());

    return 0;
}

int gooda::options::notify(){
    //No further options are allowed if not in profile mode
    auto further_options = po::collect_unrecognized(parsed_options.options, po::exclude_positional);
    if(!further_options.empty()){
        std::cerr << "Error: Unrecognized options " << further_options[0];

        for(std::size_t i = 1; i < further_options.size(); ++i){
            std::cerr << ", " << further_options[i];
        }

        std::cerr << std::endl;

        return 1;
    }

    try {
        //Must be done after the test for help/profile to handle required arguments
        po::notify(vm);
    } catch (std::exception& e ) {
        log::emit<log::Error>() << e.what() << log::endl;
        return 1;
    }

    return 0;
}
