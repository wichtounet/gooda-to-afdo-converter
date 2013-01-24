//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_CONVERTER_HPP
#define GOODA_CONVERTER_HPP

#include <boost/program_options/variables_map.hpp>

#include "afdo_data.hpp"
#include "gooda_report.hpp"

/*!
 * \file converter.hpp
 * \brief Contains the necessary functions to convert Gooda Spreadsheets 
 * to AFDO file. 
 */

namespace gooda {

/*!
 * \brief Populate the AFDO data report from the Gooda report. 
 * \param report The Gooda report
 * \param data The AFDO data. 
 * \param vm The options provided by the user. 
 */
void convert_to_afdo(const gooda_report& report, afdo_data& data, boost::program_options::variables_map& vm);

}

#endif
