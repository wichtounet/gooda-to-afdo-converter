//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "diff.hpp"
#include "logger.hpp"
#include "hash.hpp"

typedef std::pair<std::string, std::size_t> line_key;

void gooda::diff(const gooda_report& first, const gooda_report& second, boost::program_options::variables_map&){
    //Verify that both files have the same number of columsn
    if(first.get_hotspot_file().columns() != second.get_hotspot_file().columns()){
        log::emit<log::Error>() << "The two files do not have the same number of columns" << log::endl;
        return;
    }

    std::unordered_map<line_key, std::reference_wrapper<const gooda::gooda_line>> sorted_lines;

    //Sort all the source lines of the first report
    for(std::size_t i = 0; i < first.functions(); ++i){
        if(first.has_src_file(i)){
            auto& src_file = first.src_file(i);

            for(auto& line : src_file){
                line_key key = {line.get_string(src_file.column(SOURCE)), line.get_counter(src_file.column(LINE_NUMBER))};

                sorted_lines.insert(std::make_pair(key, std::cref(line)));
            }
        }
    }

    //Match sources lines of the second report with the source lines of the first report
    for(std::size_t i = 0; i < first.functions(); ++i){
        if(first.has_src_file(i)){
            auto& src_file = first.src_file(i);
            auto column = src_file.column(UNHALTED_CORE_CYCLES);

            for(auto& second_line : src_file){
                line_key key = {second_line.get_string(src_file.column(SOURCE)), second_line.get_counter(src_file.column(LINE_NUMBER))};

                auto it = sorted_lines.find(key);

                if(it != sorted_lines.end()){
                    const gooda::gooda_line& first_line = it->second;

                    std::cout << "Diff " << (first_line.get_counter(column) - second_line.get_counter(column)) << " unhalted core cycles" << std::endl;
                }
            }
        }
    }
}
