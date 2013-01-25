//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_OPTIONS_HPP
#define GOODA_OPTIONS_HPP

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
    
namespace po = boost::program_options;

namespace gooda {

/*!
 * \struct options
 * \brief Manage the command line options
 */
struct options {
    po::options_description description;    //!< The description of the options
    po::variables_map vm;                   //!< The parsed configuration
    po::parsed_options parsed_options;      //!< The parsed options

    /*!
     * \brief Construct the options
     */
    options() : description("converter [options] spreadsheets_directory"), parsed_options(&description){
        //Nothing to init
    }
    
    /*!
     * \brief Parse the command line options
     * \param argc The number of arguments
     * \param argv The arguments
     */
    int parse(int argc, const char **argv);

    /*!
     * \brief Validate the current arguments and verify that they are valid regarding to the configuration.
     */
    int notify();
};

} //end of namespace gooda

#endif
