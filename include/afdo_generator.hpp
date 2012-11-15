#ifndef GOODA_GENERATOR_HPP
#define GOODA_GENERATOR_HPP

#include <string>

#include "afdo_data.hpp"

namespace gooda {

/*!
 * \brief Generate the AFDO file corresponding to the given data. 
 * \param data The AFDO report. 
 * \param file The path to the file to write to. 
 */
void generate_afdo(const afdo_data& data, const std::string& file);

}

#endif
