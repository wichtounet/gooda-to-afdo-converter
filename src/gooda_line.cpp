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
    auto& item = m_contents.at(index);

    std::string v(item.begin(), item.end());
    std::string value = v;

    boost::trim(value);

    return value;
}

unsigned long gooda::gooda_line::get_counter(std::size_t index) const {
    auto value = get_string(index);

    return boost::lexical_cast<unsigned long>(value);
}

long gooda::gooda_line::get_address(std::size_t index) const {
    auto value = get_string(index);
    
    BOOST_ASSERT_MSG(!value.empty(), "Cannot convert and empty string to an address");

    long x = 0;   
    std::stringstream ss;
    ss << std::hex << value;
    ss >> x;

    BOOST_ASSERT_MSG(x != 0, "Address cannot be zero");

    return x;
}
