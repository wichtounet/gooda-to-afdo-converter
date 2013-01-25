//=======================================================================
// Copyright Baptiste Wicht 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*!
 * \file gcov_file.hpp
 * \brief Utility to write and read GCOV files
 */

#ifndef GOODA_GCOV_FILE_HPP
#define GOODA_GCOV_FILE_HPP

#include <iostream>
#include <fstream>

#include "gcov_types.hpp"

#define GCOV_VERSION ((gcov_unsigned_t)0x3430372a)      /* 407* */
#define GCOV_DATA_MAGIC ((gcov_unsigned_t)0x67636461)   /* "gcda" */

//AFDO tag names
#define GCOV_TAG_AFDO_FILE_NAMES ((gcov_unsigned_t)0xaa000000)
#define GCOV_TAG_AFDO_FUNCTION ((gcov_unsigned_t)0xac000000)
#define GCOV_TAG_AFDO_MODULE_GROUPING ((gcov_unsigned_t)0xae000000)
#define GCOV_TAG_AFDO_WORKING_SET ((gcov_unsigned_t)0xaf000000)

namespace gooda {

/*!
 * \class gcov_file
 * \brief Represent a GCOV File. Can be used for reading or writing GCOV files.
 */
class gcov_file {
    public:
        /*!
         * \brief Open the file for writing. 
         * \param file The path to the file.
         * \return true if the file has been successfully opened, false otherwise. 
         */
        bool open(const std::string& file);
        
        /*!
         * \brief Open the file for writing.
         * \param file The path to the file.
         * \return true if the file has been successfully opened, false otherwise. 
         */
        bool open_for_write(const std::string& file);

        /*!
         * \brief Open the file for reading. 
         * \param file The path to the file.
         * \return true if the file has been successfully opened, false otherwise. 
         */
        bool open_for_read(const std::string& file);
        
        /*!
         * \brief Write GCOV header for AFDO. 
         */
        void write_header();

        /*!
         * \brief Write header of a GCOV section. 
         * \param tag The tag of the section. 
         * \param length The lenght of the section. 
         */
        void write_section_header(gcov_unsigned_t tag, unsigned int length);

        /*!
         * \brief Write an unsigned to the file. 
         * \param value The value to write to the file. 
         */
        void write_unsigned(gcov_unsigned_t value);

        /*!
         * \brief Write a counter to the file. 
         * \param value The value to write to the file. 
         */
        void write_counter(gcov_type value);

        /*!
         * \brief Write a string to the file. 
         * \param value The value to write to the file. 
         */
        void write_string (const std::string& value);

        /*!
         * \brief Read an unsigned from the file. 
         * \return The value read from the file. 
         */
        gcov_unsigned_t read_unsigned();

        /*!
         * \brief Read a string from the file. 
         * \return The value read from the file. 
         */
        std::string read_string();

        /*!
         * \brief Read a counter from the file. 
         * \return The value read from the file. 
         */
        gcov_type read_counter();

    private:
        std::ofstream gcov_file_w;
        std::ifstream gcov_file_r;
};

}

#endif
