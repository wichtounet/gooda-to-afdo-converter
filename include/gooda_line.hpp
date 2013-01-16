//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_GOODA_LINE_HPP
#define GOODA_GOODA_LINE_HPP

#include <string>
#include <vector>

#include <boost/range/iterator_range.hpp>

/*!
 * \file gooda_line.hpp
 * \brief Contains the data structure representing a Gooda line. 
 */

/*!
 * \brief An iterator on a string
 */
typedef std::string::const_iterator string_iter;

/*!
 * \brief A pair of iterators on strings. Represent a substrings on another string. 
 */
typedef boost::iterator_range<string_iter> string_view;

namespace gooda {

/*!
 * \struct gooda_line
 * \brief A line of a Gooda Spreadsheets.
 *
 * A gooda_line is a made of the original line that was parsed and of a list
 * of positions in that line that make the columns. For performance reasons, the
 * string of each column are not extracted until it is necessary. For the same reasons, 
 * there are only converted to counter when necessary. 
 */
class gooda_line {
    public:
        /*!
         * \brief Return a string representation of the value in the given column.
         * \param index The column index. 
         * \return The string value at the given column.
         */
        std::string get_string(std::size_t index) const;
        
        /*!
         * \brief Return an integer representation of the value in the given column.
         * \param index The column index. 
         * \return The integer value at the given column.
         */
        unsigned long get_counter(std::size_t index) const;

        /*!
         * \brief Return a floating point representation of the value in the given column.
         * \param index The column index. 
         * \return The floating point value at the given column.
         */
        double get_double(std::size_t index) const;
        
        /*!
         * \brief Read an hexadecimal address and return its decimal signed value. 
         * \param index The column index
         * \return The decimal signed value of the address
         */
        long get_address(std::size_t index) const;
    
        /*!
         * \brief Returns the original line that was parsed. 
         * \return The original line that was parsed.
         */
        std::string& line();

        /*!
         * \brief Returns the original line that was parsed. 
         * \return The original line that was parsed.
         */
        const std::string& line() const;
        
        /*!
         * \brief Returns the list of the column positions. 
         * \return A std::vector containing the column positions.
         */
        std::vector<string_view>& contents();
        
        /*!
         * \brief Returns the list of the column positions. 
         * \return A std::vector containing the column positions.
         */
        const std::vector<string_view>& contents() const;

    private:
        std::string m_line;
        std::vector<string_view> m_contents;
};

} //end of namespace gooda

#endif
