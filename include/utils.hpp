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

}

#endif
