//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file afdo_printer.hpp
 * \brief Contains functions necessary to dump AFDO data. 
 */

#ifndef GOODA_AFDO_PRINTER_HPP
#define GOODA_AFDO_PRINTER_HPP

#include <string>

#include <boost/program_options/variables_map.hpp>

#include "afdo_data.hpp"

namespace gooda {

/*!
 * \brief Dump all the AFDO representation to the standard output. 
 * \param data The AFDO data. 
 * \param vm The user options.
 */
void dump_afdo(const afdo_data& data, boost::program_options::variables_map& vm);

/*!
 * \brief Dump the given afdo_stack to the standard output.
 * \param data The parent AFDO data. 
 * \param stack The inline stack to print
 * \param vm The user options.
 */
void dump_afdo(const afdo_data& data, const afdo_stack& stack, boost::program_options::variables_map& vm);

/*!
 * \brief Dump a summary of the AFDO representation to the standard output. 
 * \param data The AFDO data. 
 * \param vm The user options.
 */
void dump_afdo_light(const afdo_data& data, boost::program_options::variables_map& vm);

}

#endif
