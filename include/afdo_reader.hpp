//=======================================================================
// Copyright Baptiste Wicht 2012.
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

void read_afdo(const std::string& afdo_file, gooda::afdo_data& data, boost::program_options::variables_map& vm);

}

#endif
