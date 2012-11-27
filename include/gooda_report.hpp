#ifndef GOODA_GOODA_REPORT_HPP
#define GOODA_GOODA_REPORT_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/range/iterator_range.hpp>

/*!
 * \file gooda_report.hpp
 * \brief Contains the data structures holding the Gooda report. 
 */

typedef std::string::const_iterator string_iter;
typedef boost::iterator_range<string_iter> string_view;

#define UNHALTED_CORE_CYCLES "unhalted_core_cycles"
#define FUNCTION_NAME "Function Name"
#define FILE "Principal File"
#define DISASSEMBLY "Disassembly"
#define LINE "Line Number"

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
         * \brief Return a numeric representation of the value in the given column.
         * \param index The column index. 
         * \return The counter value at the given column.
         */
        unsigned long get_counter(std::size_t index) const;
    
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

    private:
        std::vector<gooda_line> lines;
        std::unordered_map<std::string, unsigned int> columns;
};

/*!
 * \struct gooda_report 
 * \brief The contents of a whole Gooda report. 
 */
class gooda_report {
    public:
        /*!
         * \brief return the number of hotspot functions.
         * \return the number of hotspot functions.
         */
        std::size_t functions() const;
        
        /*!
         * \brief return the number of processes. 
         * \return the number of processes.
         */
        std::size_t processes() const;

        /*!
         * \brief Create a new hotspot function. 
         * \return a new gooda_line describing a hotspot function.
         */
        gooda_line& new_hotspot_function();

        /*!
         * \brief Return the ith hotspot function. 
         * \param i the index of the function to return. 
         * \return the gooda_line representing the ith hotspot function.
         */
        const gooda_line& hotspot_function(std::size_t i) const;

        /*!
         * \brief Create a new process. 
         * \return a new gooda_line describing a process.
         */
        gooda_line& new_process();

        /*!
         * \brief Return the ith process. 
         * \param i the index of the process to return. 
         * \return the gooda_line representing the ith process.
         */
        const gooda_line& process(std::size_t i) const;
        
        /*!
         * \brief Return the source view of the ith function. 
         * 
         * If the source view does not exists, a new gooda_file is created and returned. 
         * \param i the index of the function.
         * \return the gooda_file representing the source view of the ith function. 
         */
        gooda_file& src_file(std::size_t i);
        
        /*!
         * \brief Return the source view of the ith function. 
         * 
         * If the source view does not exists, a std::out_of_range is thrown. 
         * \param i the index of the function.
         * \return the gooda_file representing the source view of the ith function. 
         */
        const gooda_file& src_file(std::size_t i) const;
        
        /*!
         * \brief Return the assembly view of the ith function. 
         * 
         * If the assembly view does not exists, a new gooda_file is created and returned. 
         * \param i the index of the function.
         * \return the gooda_file representing the assembly view of the ith function. 
         */
        gooda_file& asm_file(std::size_t i);
        
        /*!
         * \brief Return the assembly view of the ith function. 
         * 
         * If the assembly view does not exists, a std::out_of_range is thrown. 
         * \param i the index of the function.
         * \return the gooda_file representing the assembly view of the ith function. 
         */
        const gooda_file& asm_file(std::size_t i) const;

        /*!
         * \brief Indicates if the given function has a source view file. 
         * \param i the index of the function.
         * \return true if the function has a source view file, false otherwise.
         */
        bool has_src_file(std::size_t i) const;

        /*!
         * \brief Indicates if the given function has an assembly view file. 
         * \param i the index of the function.
         * \return true if the function has an assembly view file, false otherwise.
         */
        bool has_asm_file(std::size_t i) const;

        /*!
         * \brief Return the hotspot file. 
         * \return The gooda_file representing the hotspot functions.
         */
        gooda_file& get_hotspot_file();

        /*!
         * \brief Return the hotspot file. 
         * \return The gooda_file representing the hotspot functions.
         */
        const gooda_file& get_hotspot_file() const;

        /*!
         * \brief Return the process file. 
         * \return The gooda_file representing the processes.  
         */
        gooda_file& get_process_file();

        /*!
         * \brief Return the process file. 
         * \return The gooda_file representing the processes.  
         */
        const gooda_file& get_process_file() const;

    private:
        gooda_file hotspot_file;
        gooda_file process_file;
        
        std::unordered_map<std::size_t, gooda_file> src_files;
        std::unordered_map<std::size_t, gooda_file> asm_files;
};

}

#endif
