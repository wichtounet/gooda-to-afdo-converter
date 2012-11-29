#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "gooda_line.hpp"

std::string& gooda::gooda_line::line(){
    return m_line;
}

const std::string& gooda::gooda_line::line() const {
    return m_line;
}

std::vector<string_view>& gooda::gooda_line::contents(){
    return m_contents;
}

const std::vector<string_view>& gooda::gooda_line::contents() const {
    return m_contents;
}

std::string gooda::gooda_line::get_string(std::size_t index) const {
    auto& item = m_contents[index];

    std::string v(item.begin(), item.end());
    std::string value = v;

    boost::trim(value);

    return value;
}

unsigned long gooda::gooda_line::get_counter(std::size_t index) const {
    auto& item = m_contents[index];

    std::string v(item.begin(), item.end());
    std::string value = v;

    boost::trim(value);

    return boost::lexical_cast<unsigned long>(value);
}

long gooda::gooda_line::get_address(std::size_t index) const {
    auto& item = m_contents[index];

    std::string v(item.begin(), item.end());
    std::string value = v;

    boost::trim(value);

    long x;   
    std::stringstream ss;
    ss << std::hex << value;
    ss >> x;

    return x;
}
