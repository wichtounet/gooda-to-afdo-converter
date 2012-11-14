#ifndef CONVERTER_READER_HPP
#define CONVERTER_READER_HPP

#include <string>

#include "afdo_data.hpp"
#include "gooda_report.hpp"

namespace converter {

void read_report(const gooda_report& report, afdo_data& data);

}

#endif
