#ifndef CONVERTER_GOODA_READER_HPP
#define CONVERTER_GOODA_READER_HPP

#include <string>

#include "gooda_report.hpp"

namespace gooda {

//TODO It would probably be necessary to avoid the copy implied by return

gooda_report read_spreadsheets(const std::string& directory);

}

#endif
