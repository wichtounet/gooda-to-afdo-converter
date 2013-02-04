//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file afdo_diff.hpp
 * \brief Contains the necessary functions to perform diff between two AFDO profiles. 
 */

#ifndef GOODA_AFDO_DIFF_HPP
#define GOODA_AFDO_DIFF_HPP

#include <boost/program_options/variables_map.hpp>

#include "afdo_data.hpp"

namespace gooda {

/*!
 * \brief Perform the diff two AFDO profiles.
 * \param first The first AFDO profile
 * \param second The second AFDO profile
 * \param vm The configuration, used to configure the printings
 */
void afdo_diff(const afdo_data& first, const afdo_data& second, boost::program_options::variables_map& vm);

}

#endif
