//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file gooda_reader.hpp
 * \brief Contains functions necessary to read Gooda spreadsheets. 
 */

#ifndef GOODA_GOODA_READER_HPP
#define GOODA_GOODA_READER_HPP

#include <string>

#include "gooda_report.hpp"

namespace gooda {

/*!
 * \brief Read the Gooda spreadsheets and populate the Gooda report
 * \param directory The spreadsheets directory to read. 
 * \return The populated Gooda report. 
 */
gooda_report read_spreadsheets(const std::string& directory);

}

#endif
