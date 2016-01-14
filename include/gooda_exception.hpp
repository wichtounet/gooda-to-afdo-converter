//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file gooda_exception.hpp
 * \brief exception used by all parts of the converter to perform error handling.
 */

#ifndef GOODA_EXCEPTION_H
#define GOODA_EXCEPTION_H

#include <string>

namespace gooda {

/*!
 * \class gooda_exception
 * \brief An exception occurring during the parsing of the spreadsheets
 */
class gooda_exception: public std::exception {
    protected:
        std::string m_message; //!< The exception message

    public:
        /*!
         * \brief Construct an exception
         * \param message The message to be displayed.
         */
        explicit gooda_exception(const std::string& message);

        /*!
         * \brief Destruct the exception
         */
        ~gooda_exception() throw();

        /*!
         * \brief Return the message of the exception to be displayed by the system
         * \return The exception details.
         */
        virtual const char* what() const throw();
};

} //end of gooda

#endif
