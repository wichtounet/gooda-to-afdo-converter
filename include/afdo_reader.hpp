//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_AFDO_READER_HPP
#define GOODA_AFDO_READER_HPP

#include <string>

#include <boost/program_options/variables_map.hpp>

#include "afdo_data.hpp"

/*!
 * \file afdo_reader.hpp
 * \brief Contains the necessary functions to read AFDO profile. 
 */

namespace gooda {

/*!
 * \brief Read an AFDO file and populate an AFDO data structure with its data. 
 * \param afdo_file The path to the AFDO file. 
 * \param data The data to populate. 
 * \param vm The user configuration. 
 */
void read_afdo(const std::string& afdo_file, gooda::afdo_data& data, boost::program_options::variables_map& vm);

}

#endif
