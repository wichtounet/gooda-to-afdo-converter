#ifndef GOODA_READER_HPP
#define GOODA_READER_HPP

#include <string>

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
void read_report(const gooda_report& report, afdo_data& data, boost::program_options::variables_map& vm);

}

#endif
