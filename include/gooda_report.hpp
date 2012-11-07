#ifndef CONVERTER_GOODA_REPORT_HPP
#define CONVERTER_GOODA_REPORT_HPP

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

typedef std::string::const_iterator string_iter;
typedef boost::iterator_range<string_iter> string_view;

namespace converter {

struct gooda_line {
    std::string line;
    std::vector<string_view> contents;
};

class gooda_report {
    public:
        std::size_t functions() const;

        void add_hotspot_function(gooda_line&& line);

    private:
        std::vector<gooda_line> hotspot_functions;
};

}

#endif
