#ifndef GOODA_AFDO_PRINTER_HPP
#define GOODA_AFDO_PRINTER_HPP

#include <string>

#include "afdo_data.hpp"

/*!
 * \file afdo_printer.hpp
 * \brief Contains functions necessary to dump AFDO data. 
 */

namespace gooda {

/*!
 * \brief Dump all the AFDO representation to the standard output
 * \param data The AFDO data. 
 */
void dump_afdo(const afdo_data& data);

void dump_afdo_light(const afdo_data& data);

}

#endif
