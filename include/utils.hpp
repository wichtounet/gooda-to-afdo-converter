#ifndef GOODA_UTILS_HPP
#define GOODA_UTILS_HPP

#include <string>

/*!
 * \file utils.hpp
 * \brief Contains utility functions. 
 */

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
 * \brief Execute a command and return the output of the command. 
 * \param command The command to execute.  
 * \return The standard output of the executed command. 
 */
std::string exec_command(const std::string& command);

int processor_model();

}

#endif
