#ifndef CONVERTER_GENERATOR_HPP
#define CONVERTER_GENERATOR_HPP

#include <string>

#include "data.hpp"

namespace converter {

void generate_afdo(const afdo_data& data, const std::string& file);

}

#endif
