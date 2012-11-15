#ifndef GOODA_GOODA_READER_HPP
#define GOODA_GOODA_READER_HPP

#include <string>

#include "gooda_report.hpp"

namespace gooda {

//TODO It would probably be necessary to avoid the copy implied by return

/*!
 * \brief Read the Gooda spreadsheets and populate the Gooda report
 * \param directory The spreadsheets directory to read. 
 * \return The populated Gooda report. 
 */
gooda_report read_spreadsheets(const std::string& directory);

}

#endif
