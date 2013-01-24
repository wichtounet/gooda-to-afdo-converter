//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_EXCEPTION_H
#define GOODA_EXCEPTION_H

#include <string>

namespace gooda {

class gooda_exception: public std::exception {
    protected:
        std::string m_message;

    public:
        gooda_exception(const std::string& message);
        ~gooda_exception() throw();

        virtual const char* what() const throw();
};

} //end of gooda

#endif
