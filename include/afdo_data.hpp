//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file afdo_data.hpp
 * \brief Contains the data structures that are used to hold AFDO data. 
 */

#ifndef GOODA_DATA_HPP
#define GOODA_DATA_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <array>

#include "gcov_types.hpp"

namespace gooda {

/*!
 * \var WS_SIZE
 * \brief The size of the Working Set Histogram. 
 */
static const unsigned int WS_SIZE = 128;

/*!
 * \struct afdo_pos
 * \brief The position of an instruction. 
 */
struct afdo_pos {
    std::string func;                   //!< Source function 
    std::string file;                   //!< Source file
    gcov_unsigned_t line;               //!< Source line
    gcov_unsigned_t discriminator;      //!< The DWARF discriminator

    /*!
     * \brief Construct a new afdo_pos. 
     * \param func The source function
     * \param file The source file
     * \param line The source line
     * \param discriminator The DWARF discriminator
     */
    afdo_pos(const std::string& func, const std::string& file, gcov_unsigned_t line, gcov_unsigned_t discriminator)
        : func(func), file(file), line(line), discriminator(discriminator) {};

    /*!
     * \brief Compare two afdo_pos
     * \param rhs The afdo_pos to compare to. 
     * \return true if both afdo_pos are equals, false otherwise
     */
    bool operator==(const afdo_pos& rhs) const ;

    /*!
     * \brief Compare two afdo_pos
     * \param rhs The afdo_pos to compare to. 
     * \return true if both afdo_pos are not equals, false otherwise
     */
    bool operator!=(const afdo_pos& rhs) const ;
};

/*!
 * \struct afdo_stack
 * \brief An inline stack. 
 */
struct afdo_stack {
    std::vector<afdo_pos> stack;    //!< The stack of source position (the inline stack)
    gcov_type count = 0;            //!< The value of the counter for this inline stack
    gcov_type cache_misses = 0;     //!< The number of cache misses in this inline stack
    gcov_type num_inst = 0;         //!< The number of dynamic instructions in this inline stack
};

/*!
 * \struct afdo_function
 * \brief Structure holding the profile of a function. 
 */
struct afdo_function {
    std::string name = "";              //!< The name of the function, mangled
    std::string file = "";              //!< The source file of this function
    gcov_type total_count = 0;          //!< The total number of counts inside this function
    gcov_type entry_count = 0;          //!< The count of the entry basic block
    std::vector<afdo_stack> stacks;     //!< The inline stacks of the function

    //Not generated in AFDO
    std::size_t i;                      //!< The position in the Gooda spreadsheets
    std::string executable_file;        //!< The ELF file this function is contained in
};

/*!
 * \struct afdo_module
 * \brief Structure holding the information of module. 
 */
struct afdo_module {
    std::string name = "";                  //!< The name of the module, the name of the module file
    gcov_unsigned_t exported = 0;           //!< Flag indicating if the module is exported
    gcov_unsigned_t has_asm = 0;            //!< Flag indicating if the module contains asm
    gcov_unsigned_t num_aux_modules = 0;    //!< The number of auxiliary modules
    gcov_unsigned_t num_quote_paths = 0;    //!< The number of quote paths
    gcov_unsigned_t num_bracket_paths = 0;  //!< The number of bracket paths
    gcov_unsigned_t num_cpp_defines = 0;    //!< The number of cpp defines
    gcov_unsigned_t num_cpp_includes = 0;   //!< The number of cpp includes
    gcov_unsigned_t num_cl_args = 0;        //!< The number of arguments
    std::vector<std::string> strings;       //!< The strings for the previous num_xxx members
};

/*!
 * \struct afdo_working_set
 * \brief Structure holding one bucket of the working set. 
 */
struct afdo_working_set {
    gcov_unsigned_t num_counter;            //!< The number of counters
    gcov_type min_counter;                  //!< The minimal number of counters
};

/*!
 * \struct afdo_data
 * \brief Structure holding the AFDO profile
 */
struct afdo_data {
    std::vector<std::string> file_names;                    //!< The file name index
    std::vector<afdo_function> functions;                   //!< The function profile
    std::vector<afdo_module> modules;                       //!< All the modules
    std::array<afdo_working_set, WS_SIZE> working_set;      //!< The Working Set

    unsigned int length_file_section = 0;                   //!< The length of the file section
    unsigned int length_function_section = 0;               //!< The length of the function section
    unsigned int length_modules_section = 0;                //!< The length of the modules section
    unsigned int length_working_set_section = 0;            //!< The length of the working set section

    afdo_data(){}

    /*!
     * \brief Deleted copy constructor
     * \param other The other afdo_data
     */
    afdo_data(const afdo_data& other) = delete;
    
    /*!
     * \brief Deleted copy assignment operator
     * \param other The other afdo_data
     * \return A reference to this
     */
    afdo_data& operator=(const afdo_data& other) = delete;

    /*!
     * \brief Return the index of the given file. Also working for strings. 
     *
     * Throws an exception if the file does not exists. See add_file_name to add the string/file to the index before. 
     * \sa afdo_data::add_file_name(const std::string&)
     *
     * \param file The file name. 
     * \return The file index. 
     */
    gcov_unsigned_t get_file_index(const std::string& file) const;

    /*!
     * \brief Return the file name with the given index. 
     * \param i The index of the of file name to search. 
     * \return The file name. 
     */
    const std::string& file_name(std::size_t i) const;

    /*!
     * \brief Add a file name to the string/file index.
     * \param file The string/file to add to the index.
     */
    void add_file_name(const std::string& file);
    
    private:
        std::unordered_map<std::string, std::size_t> file_index;
};

}

#endif
