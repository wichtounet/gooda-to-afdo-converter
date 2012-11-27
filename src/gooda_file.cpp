#include "gooda_file.hpp"

gooda::gooda_line& gooda::gooda_file::new_line(){
    int i = lines.size();

    lines.resize(i + 1);

    return lines[i];
}

std::size_t gooda::gooda_file::size() const {
    return lines.size();
}

gooda::gooda_line& gooda::gooda_file::line(std::size_t i){
    return lines[i];
}

const gooda::gooda_line& gooda::gooda_file::line(std::size_t i) const {
    return lines[i];
}

gooda::gooda_file::iterator gooda::gooda_file::begin(){
    return lines.begin();
}

gooda::gooda_file::iterator gooda::gooda_file::end(){
    return lines.end();
}

gooda::gooda_file::const_iterator gooda::gooda_file::begin() const {
    return lines.cbegin();
}

gooda::gooda_file::const_iterator gooda::gooda_file::end() const {
    return lines.cend();
}

unsigned int& gooda::gooda_file::column(const std::string& column){
    return columns[column];
}

int gooda::gooda_file::column(const std::string& column) const {
    return columns.at(column);
}
