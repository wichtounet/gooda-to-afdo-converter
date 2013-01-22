//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "gooda_exception.hpp"

gooda::gooda_exception::gooda_exception(const std::string& message) : m_message(message) {}

gooda::gooda_exception::~gooda_exception() throw() {}

const char* gooda::gooda_exception::what() const throw() {
    return m_message.c_str();
}
