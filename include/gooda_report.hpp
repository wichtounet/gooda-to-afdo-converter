//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_GOODA_REPORT_HPP
#define GOODA_GOODA_REPORT_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include "gooda_file.hpp"
#include "gooda_line.hpp"

/*!
 * \file gooda_report.hpp
 * \brief Contains the data structures holding the Gooda report. 
 */

#define UNHALTED_CORE_CYCLES "unhalted_core_cycles"
#define BB_EXEC "BB_Exec"

#define FUNCTION_NAME "Function Name"
#define LINE "Line Number"

//For ASM
#define PRINC_FILE "Principal File"
#define PRINC_LINE "Princ_L#"
#define INIT_LINE "Init_L#"
#define INIT_FILE "Initial File"
#define DISASSEMBLY "Disassembly"

//For hotspot functions
#define OFFSET "Offset"
#define LENGTH "Length"
#define ADDRESS "Address"
#define MODULE "Module"
#define PROCESS "Process"

//For process view
#define PROCESS_PATH "Process Path"

namespace gooda {

/*!
 * \struct gooda_report 
 * \brief The contents of a whole Gooda report. 
 */
class gooda_report {
    public:
        /*!
         * \brief return the number of hotspot functions.
         * \return the number of hotspot functions.
         */
        std::size_t functions() const;
        
        /*!
         * \brief return the number of processes. 
         * \return the number of processes.
         */
        std::size_t processes() const;

        /*!
         * \brief Create a new hotspot function. 
         * \return a new gooda_line describing a hotspot function.
         */
        gooda_line& new_hotspot_function();

        /*!
         * \brief Return the ith hotspot function. 
         * \param i the index of the function to return. 
         * \return the gooda_line representing the ith hotspot function.
         */
        const gooda_line& hotspot_function(std::size_t i) const;

        /*!
         * \brief Create a new process. 
         * \return a new gooda_line describing a process.
         */
        gooda_line& new_process();

        /*!
         * \brief Return the ith process. 
         * \param i the index of the process to return. 
         * \return the gooda_line representing the ith process.
         */
        const gooda_line& process(std::size_t i) const;
        
        /*!
         * \brief Return the source view of the ith function. 
         * 
         * If the source view does not exists, a new gooda_file is created and returned. 
         * \param i the index of the function.
         * \return the gooda_file representing the source view of the ith function. 
         */
        gooda_file& src_file(std::size_t i);
        
        /*!
         * \brief Return the source view of the ith function. 
         * 
         * If the source view does not exists, a std::out_of_range is thrown. 
         * \param i the index of the function.
         * \return the gooda_file representing the source view of the ith function. 
         */
        const gooda_file& src_file(std::size_t i) const;
        
        /*!
         * \brief Return the assembly view of the ith function. 
         * 
         * If the assembly view does not exists, a new gooda_file is created and returned. 
         * \param i the index of the function.
         * \return the gooda_file representing the assembly view of the ith function. 
         */
        gooda_file& asm_file(std::size_t i);
        
        /*!
         * \brief Return the assembly view of the ith function. 
         * 
         * If the assembly view does not exists, a std::out_of_range is thrown. 
         * \param i the index of the function.
         * \return the gooda_file representing the assembly view of the ith function. 
         */
        const gooda_file& asm_file(std::size_t i) const;

        /*!
         * \brief Indicates if the given function has a source view file. 
         * \param i the index of the function.
         * \return true if the function has a source view file, false otherwise.
         */
        bool has_src_file(std::size_t i) const;

        /*!
         * \brief Indicates if the given function has an assembly view file. 
         * \param i the index of the function.
         * \return true if the function has an assembly view file, false otherwise.
         */
        bool has_asm_file(std::size_t i) const;

        /*!
         * \brief Return the hotspot file. 
         * \return The gooda_file representing the hotspot functions.
         */
        gooda_file& get_hotspot_file();

        /*!
         * \brief Return the hotspot file. 
         * \return The gooda_file representing the hotspot functions.
         */
        const gooda_file& get_hotspot_file() const;

        /*!
         * \brief Return the process file. 
         * \return The gooda_file representing the processes.  
         */
        gooda_file& get_process_file();

        /*!
         * \brief Return the process file. 
         * \return The gooda_file representing the processes.  
         */
        const gooda_file& get_process_file() const;

    private:
        gooda_file hotspot_file;
        gooda_file process_file;
        
        std::unordered_map<std::size_t, gooda_file> src_files;
        std::unordered_map<std::size_t, gooda_file> asm_files;
};

} //end of namespace gooda

#endif
