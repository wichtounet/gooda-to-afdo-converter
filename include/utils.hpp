#ifndef CONVERTER_UTILS_HPP
#define CONVERTER_UTILS_HPP

#include <string>

namespace converter {

bool exists(const std::string& file);
bool is_directory(const std::string& file);

}

#endif
