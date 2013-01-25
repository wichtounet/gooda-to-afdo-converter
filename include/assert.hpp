//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_ASSERT_HPP
#define GOODA_ASSERT_HPP

/*!
 * \file assert.hpp
 * \brief Contains assertion functions. 
 */

#include <cassert>

#include <boost/assert.hpp>

//For older versions of Boost

#ifndef BOOST_ASSERT_MSG

#define BOOST_ASSERT_MSG(condition, message) BOOST_ASSERT(condition)

#endif

/*!
 * \def gooda_assert(condition, message)
 * \brief Verify that the condition is true. If not, fails and display the specified message. 
 * \param condition The condition that have to be true
 * \param message The message to be printed if the assertion is not verified. 
 */
#define gooda_assert(condition, message) BOOST_ASSERT_MSG(condition, message);

#ifdef __GNUC__

/*!
 * \def gooda_unreachable(message)
 * \brief Assert that this path is not taken. If it is taken, fails and display the specified message. 
 * \param message The message to be printed if the assertion is not verified. 
 */
#define gooda_unreachable(message) BOOST_ASSERT_MSG(false, message); assert(false); __builtin_unreachable();

#else

/*!
 * \def gooda_unreachable(message)
 * \brief Assert that this path is not taken. If it is taken, fails and display the specified message. 
 * \param message The message to be printed if the assertion is not verified. 
 */
#define gooda_unreachable(message) BOOST_ASSERT_MSG(false, message); assert(false);

#endif

#endif
