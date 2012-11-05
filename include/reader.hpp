#ifndef CONVERTER_READER_HPP
#define CONVERTER_READER_HPP

#include <string>

#include "data.hpp"

namespace converter {

bool read_spreadsheets(const std::string& directory, Data& data);

}

#endif
