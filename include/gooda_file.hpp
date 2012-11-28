#ifndef GOODA_GOODA_FILE_HPP
#define GOODA_GOODA_FILE_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include "gooda_line.hpp"

/*!
 * \file gooda_file.hpp
 * \brief Contains the data structures holding a Gooda file. 
 */

namespace gooda {

/*!
 * \struct gooda_file
 * \brief The contents of a specific Gooda file. 
 */
class gooda_file {
    public:
        /*!
         * \brief Iterator on the lines of the file.
         */
        typedef std::vector<gooda_line>::iterator iterator;
        
        /*!
         * \brief Const-iterator on the lines of the file.
         */
        typedef std::vector<gooda_line>::const_iterator const_iterator;

        /*!
         * \brief Return an iterator to the first line of the file. 
         * \return An iterator to the first line of the file.
         */
        iterator begin();

        /*!
         * \brief Return an iterator one past the last line of the file. 
         * \return An iterator one past the last line of the file. 
         */
        iterator end();

        /*!
         * \brief Return an iterator to the first line of the file. 
         * \return An iterator to the first line of the file.
         */
        const_iterator begin() const;

        /*!
         * \brief Return an iterator one past the last line of the file. 
         * \return An iterator one past the last line of the file. 
         */
        const_iterator end() const;
        
        /*!
         * \brief Creates and returns a new line.
         * \return A newly created gooda_line. 
         */
        gooda_line& new_line();

        /*!
         * \brief Returns the number of line of the file.
         * \return the number of line of the file.
         */
        std::size_t size() const;
        
        /*!
         * \brief Return the ith line of the file.
         * \param i The index of the line to return. 
         * \return The ith gooda_line.
         */
        gooda_line& line(std::size_t i);
        
        /*!
         * \brief Return the ith line of the file.
         * \param i The index of the line to return. 
         * \return The ith gooda_line.
         */
        const gooda_line& line(std::size_t i) const;

        /*!
         * \brief Return the index at which the given column is. 
         * \param column The textual name of the column ("Disassembly for instance")
         * \return The index of the column.
         */
        unsigned int& column(const std::string& column);

        /*!
         * \brief Return the index at which the given column is. 
         * \param column The textual name of the column ("Disassembly for instance")
         * \return The index of the column.
         */
        int column(const std::string& column) const;

        /*!
         * \brief Return the number of lines of the file
         * \return the number of lines of the file. 
         */
        std::size_t lines() const;

        /*!
         * \brief Return the number of columns of the file
         * \return the number of columns of the file. 
         */
        std::size_t columns() const;

    private:
        std::vector<gooda_line> m_lines;
        std::unordered_map<std::string, unsigned int> m_columns;
};

} //end of namespace gooda

#endif
