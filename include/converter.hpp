#ifndef GOODA_READER_HPP
#define GOODA_READER_HPP

#include <string>

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
 */
void read_report(const gooda_report& report, afdo_data& data);

}

#endif