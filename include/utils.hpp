//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file utils.hpp
 * \brief Contains utility functions. 
 */

#ifndef GOODA_UTILS_HPP
#define GOODA_UTILS_HPP

#include <string>

namespace gooda {

/*!
 * \brief Test if a file exists.
 * \param file The file to test.
 * \return true if the file exists, false otherwise. 
 */
bool exists(const std::string& file);

/*!
 * \brief Test if a file is a directory. 
 * \param file The file to test. 
 * \return true if the file is a directory, false otherwise. 
 */
bool is_directory(const std::string& file);

/*!
 * \brief Execute a command and return the return code of the command. 
 * \param command The command to execute.  
 * \return The return code of the command. 
 */
int exec_command(const std::string& command);

/*!
 * \brief Execute a command and return the its output.
 * \param command The command to execute.  
 * \return The output of the command. 
 */
std::string exec_command_result(const std::string& command);

/*!
 * \return The model of the current processor. 
 * \return The model of the current processor or -1 if not able to find it. 
 */
int processor_model();

}

#endif
