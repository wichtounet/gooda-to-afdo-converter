//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_GCOV_TYPES_HPP
#define GOODA_GCOV_TYPES_HPP

/*!
 * \file gcov_types.hpp
 * \brief Contains the typedef for the GCOV types. 
 */

/*!
 * \brief Type to hold most of data in GCOV. 
 */
typedef unsigned int gcov_unsigned_t;

/*!
 * \brief Type to hold the value of a counter
 */
typedef unsigned long gcov_type;

static_assert(sizeof(gcov_unsigned_t) == 4, "A gcov_unsigned_t must be 32 bits");
static_assert(sizeof(gcov_type) == 8, "A gcov_type must be 64 bits");

#endif
