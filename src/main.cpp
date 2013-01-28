//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file main.cpp
 * \brief Implementation of the interpreter of the options and execution of the correct functions for each use case. 
 */

#include <iostream>
#include <chrono>

#include "utils.hpp"
#include "gooda_reader.hpp"
#include "converter.hpp"
#include "afdo_generator.hpp"
#include "afdo_printer.hpp"
#include "afdo_reader.hpp"
#include "logger.hpp"
#include "diff.hpp"
#include "afdo_diff.hpp"
#include "Options.hpp"
#include "gooda_exception.hpp"

namespace {

/*!
 * \typedef Clock
 * High precision clock of std::chrono
 */
typedef std::chrono::high_resolution_clock Clock;

/*!
 * \typedef milliseconds
 * A milliseconds duration
 */
typedef std::chrono::milliseconds milliseconds;

/*!
 * \brief Process the Gooda spreadsheets
 * \param directory The spreadsheets directory
 * \param vm The configuration
 */
void process_spreadsheets(const std::string& directory, po::variables_map& vm){
    Clock::time_point t0 = Clock::now();

    //Read the Gooda Spreadsheets
    auto report = gooda::read_spreadsheets(directory);

    gooda::afdo_data data;

    //Convert the Gooda report to AFDO
    gooda::convert_to_afdo(report, data, vm);

    //Execute the specified action
    if(vm.count("dump")){
        gooda::dump_afdo_light(data, vm);
    } else if(vm.count("full-dump")){
        gooda::dump_afdo(data, vm);
    } else {
        gooda::generate_afdo(data, vm["output"].as<std::string>(), vm);
    }

    Clock::time_point t1 = Clock::now();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - t0);

    log::emit<log::Debug>() << "Conversion took " << ms.count() << "ms" << log::endl;
}

/*!
 * \brief Generate the difference between two sets of spreadsheets
 * \param first The path to the first spreadsheets directory. 
 * \param second The path to the second spreadsheets directory.
 * \param vm The configuration
 */
void diff(const std::string& first, const std::string& second, po::variables_map& vm){
    Clock::time_point t0 = Clock::now();

    //Read the Gooda Spreadsheets
    auto first_report = gooda::read_spreadsheets(first);
    auto second_report = gooda::read_spreadsheets(second);

    diff(first_report, second_report, vm);
    
    Clock::time_point t1 = Clock::now();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - t0);

    log::emit<log::Debug>() << "Diff took " << ms.count() << "ms" << log::endl;
}

void afdo_diff(const std::string& first, const std::string& second, po::variables_map& vm){
    Clock::time_point t0 = Clock::now();
    
    gooda::afdo_data data_first;
    gooda::afdo_data data_second;
    
    gooda::read_afdo(first, data_first, vm);
    gooda::read_afdo(second, data_second, vm);

    afdo_diff(data_first, data_second, vm);
    
    Clock::time_point t1 = Clock::now();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - t0);

    log::emit<log::Debug>() << "Diff took " << ms.count() << "ms" << log::endl;
}

/*!
 * \brief Read an AFDO file
 * \param afdo_file The path to the AFDO file
 * \param vm The configuration
 */
void process_afdo(const std::string& afdo_file, po::variables_map& vm){
    Clock::time_point t0 = Clock::now();

    log::emit<log::Debug>() << "Start reading " << afdo_file << log::endl;
    
    gooda::afdo_data data;

    //Read the AFDO file into data structure
    gooda::read_afdo(afdo_file, data, vm);

    if(vm.count("dump")){
        gooda::dump_afdo_light(data, vm);
    } 
    //The default option is to print the full dump
    else {
        gooda::dump_afdo(data, vm);
    }
    
    Clock::time_point t1 = Clock::now();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
    
    log::emit<log::Debug>() << "Conversion took " << ms.count() << "ms" << log::endl;
}

/*!
 * \brief Profile the application given on the command line
 * \param vm The configuration
 * \param parsed_options The parsed options. Used to get the parameters of the application to run
 * \return An integer code indicating the status of the profiling.
 */
void profile_application(po::variables_map& vm, po::parsed_options& parsed_options){
    int processor_model = -1;

    if(vm.count("model")){
        processor_model = vm["model"].as<int>();
    } else {
        processor_model = gooda::processor_model();

        if(processor_model == -1){
            throw gooda::gooda_exception("Cannot find your processor model. Please provide it with the --model option. ");
        }
    }

    std::string script;

    if(processor_model == 0x2A || processor_model == 0x2D){
        log::emit<log::Debug>() << "Detected processor as \"Sandy Bridge\"" << log::endl;
        script = "run_record_cyc_snb.sh";
    } else if(processor_model == 0x3A){
        log::emit<log::Debug>() << "Detected processor as \"Ivy Bridge\"" << log::endl;
        script = "run_record_cyc_ivb.sh";
    } else if(processor_model == 0x25 || processor_model == 0x2C || processor_model == 0x2F){
        log::emit<log::Debug>() << "Detected processor as \"Westmere\"" << log::endl;
        script = "run_record_cyc_wsm_ep.sh";
    } else {
        throw gooda::gooda_exception("Sorry, your processor is not supported by Gooda. \n Only Westmere, Sandy Bridge and Ivy Bridge are currently supported");
    }

    //There is only one script for LBR
    if(vm.count("lbr")){
        script = "gooda_bb_exec.sh";
    }

    //Use the special scripts for benchmarking
    if(vm.count("bench")){
        script = "wichtounet_" + script;
    }

    //Try to find the Gooda directory
    std::string gooda_dir;
    const char* gooda_dir_val = ::getenv("GOODA_DIR");
    if(!gooda_dir_val){
        gooda_dir = "";
    } else {
        gooda_dir = gooda_dir_val;
    }

    std::string profile_command;
    if(vm.count("gooda")){
        profile_command = "sudo bash " + vm["gooda"].as<std::string>() + "/scripts/" + script + " ";
    } else if(gooda_dir.empty()){
        profile_command = "sudo bash scripts/" + script + " ";
    } else {
        profile_command = "sudo bash " + gooda_dir + "/scripts/" + script + " ";
    }

    auto further_options = po::collect_unrecognized(parsed_options.options, po::include_positional);

    //Append the application
    for(auto& option : further_options){
        profile_command += option + " ";
    }

    log::emit<log::Debug>() << "Profile the given application (perf needs to be run in root)" << log::endl;
    log::emit<log::Debug>() << "Command: \"" << profile_command << "\"" << log::endl;

    gooda::exec_command(profile_command);

    std::string gooda_command;
    if(vm.count("gooda")){
        gooda_command = "sudo " + vm["gooda"].as<std::string>() + "/gooda";
    } else if(gooda_dir.empty()){
        gooda_command = "sudo ./gooda";
    } else {
        gooda_command = "sudo " + gooda_dir + "/gooda";
    }

    if(vm.count("quiet")){
        gooda_command += " > /dev/null 2> /dev/null";
    }

    log::emit<log::Debug>() << "Run Gooda (Gooda needs to be run in root)" << log::endl;
    log::emit<log::Debug>() << "Command: \"" << gooda_command << "\"" << log::endl;

    gooda::exec_command(gooda_command);

    //If no output action is specified, just as as a wrapper of Gooda
    if(vm.count("afdo") || vm.count("dump") || vm.count("full-dump")){
        //Process the spreadsheets to generate AFDO
        process_spreadsheets("spreadsheets", vm); 
    }
}

} //end of anonymous namespace

int main(int argc, const char **argv){
    try {
        gooda::options options;
        options.parse(argc, argv);

        //Profiling mode
        if(options.vm.count("profile")){
            profile_application(options.vm, options.parsed_options);
            return 0;
        }

        options.notify();

        auto& vm = options.vm;

        if(!vm.count("input-file")){
            log::emit<log::Error>() << "No file provided" << log::endl;

            return 1;
        }

        auto input_files = vm["input-file"].as<std::vector<std::string>>();

        //Test that there is a least one file
        if(input_files.empty()){
            log::emit<log::Error>() << "No file provided" << log::endl;

            return 1;
        }

        if(vm.count("diff")){
            //Verify that there are two difrections
            if(input_files.size() != 2){
                log::emit<log::Error>() << "Two directories are necessary to perform a diff" << log::endl;

                return 1;
            }

            std::string first = input_files[0];
            std::string second = input_files[1];

            //The directories must exists
            if(!gooda::exists(first)){
                log::emit<log::Error>() << "\"" << first << "\" does not exists" << log::endl;
                return 1;
            }

            //The directories must exists
            if(!gooda::exists(second)){
                log::emit<log::Error>() << "\"" << second << "\" does not exists" << log::endl;
                return 1;
            }

            //The file must be a directory 
            if(!gooda::is_directory(first)){
                log::emit<log::Error>() << "\"" << first << "\" is not a directory" << log::endl;
                return 1;
            }

            //The file must be a directory 
            if(!gooda::is_directory(second)){
                log::emit<log::Error>() << "\"" << second << "\" is not a directory" << log::endl;
                return 1;
            }

            //Perform the diff
            diff(first, second, vm);

            return 0;
        } else {
            //Verify that only one directory is provided
            if(input_files.size() > 1){
                log::emit<log::Error>() << "Only one file can be analyzed at a time" << log::endl;

                return 1;
            }

            std::string input_file = input_files[0];

            //The file must exists
            if(!gooda::exists(input_file)){
                log::emit<log::Error>() << "\"" << input_file << "\" does not exists" << log::endl;
                return 1;
            }

            if(vm.count("read-afdo")){
                process_afdo(input_file, vm); 
            } 
            //By default, read spreadsheets
            else {
                //The file must be a directory 
                if(!gooda::is_directory(input_file)){
                    log::emit<log::Error>() << "\"" << input_file << "\" is not a directory" << log::endl;
                    return 1;
                }

                process_spreadsheets(input_file, vm);
            }
        }
    } catch (const gooda::gooda_exception& e){
        log::emit<log::Error>() << e.what() << log::endl;

        return -1;
    }

    return 0;
}
