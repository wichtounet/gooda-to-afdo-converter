//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file gooda_file.cpp
 * \brief Implementation of gooda_file. 
 */

#include "gooda_file.hpp"

gooda::gooda_line& gooda::gooda_file::new_line(){
    int i = m_lines.size();

    m_lines.resize(i + 1);

    return m_lines.at(i);
}

gooda::gooda_line& gooda::gooda_file::multiplex_line(){
    return m_multiplex_line;
}

const gooda::gooda_line& gooda::gooda_file::multiplex_line() const {
    return m_multiplex_line;
}

std::size_t gooda::gooda_file::size() const {
    return m_lines.size();
}

gooda::gooda_line& gooda::gooda_file::line(std::size_t i){
    return m_lines.at(i);
}

const gooda::gooda_line& gooda::gooda_file::line(std::size_t i) const {
    return m_lines.at(i);
}

gooda::gooda_file::iterator gooda::gooda_file::begin(){
    return m_lines.begin();
}

gooda::gooda_file::iterator gooda::gooda_file::end(){
    return m_lines.end();
}

gooda::gooda_file::const_iterator gooda::gooda_file::begin() const {
    return m_lines.cbegin();
}

gooda::gooda_file::const_iterator gooda::gooda_file::end() const {
    return m_lines.cend();
}

unsigned int& gooda::gooda_file::column(const std::string& column_name){
    return m_columns[column_name];
}
        
bool gooda::gooda_file::has_column(const std::string& column_name) const {
    return m_columns.find(column_name) != m_columns.end();
}

unsigned int gooda::gooda_file::column(const std::string& column_name) const {
    return m_columns.at(column_name);
}

std::size_t gooda::gooda_file::lines() const {
    return m_lines.size();
}

std::size_t gooda::gooda_file::columns() const {
    return m_columns.size();
}
