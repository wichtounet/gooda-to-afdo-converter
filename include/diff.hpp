//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file diff.hpp
 * \brief Contains the necessary functions to perform diff between two sets of spreadsheets. 
 */

#ifndef GOODA_DIFF_HPP
#define GOODA_DIFF_HPP

#include <boost/program_options/variables_map.hpp>

#include "gooda_report.hpp"

namespace gooda {

/*!
 * \brief Performs a diff between two Gooda reports. 
 * \param first_report The first Gooda report
 * \param second_report The second Gooda report. 
 * \param vm The options provided by the user. 
 */
void diff(const gooda_report& first_report, const gooda_report& second_report, boost::program_options::variables_map& vm);

}

#endif
