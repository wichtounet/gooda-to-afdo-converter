#ifndef CONVERTER_GOODA_REPORT_HPP
#define CONVERTER_GOODA_REPORT_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

typedef std::string::const_iterator string_iter;
typedef boost::iterator_range<string_iter> string_view;

namespace converter {

struct gooda_line {
    std::string line;
    std::vector<string_view> contents;
};

struct gooda_file {
    std::vector<gooda_line> lines;
};

class gooda_report {
    public:
        std::size_t functions() const;

        void add_hotspot_function(gooda_line&& line);

        gooda_file& src_file(std::size_t i);
        gooda_file& asm_file(std::size_t i);

    private:
        gooda_file hotspot_file;
        
        std::unordered_map<std::size_t, gooda_file> src_files;
        std::unordered_map<std::size_t, gooda_file> asm_files;
};

}

#endif
