#ifndef OPTION_H
#define OPTION_H

// Alias for boost::optional, can be changed to std::optional when C++17 is used
#include <boost/optional.hpp>

// For output via operator<<
#include <boost/optional/optional_io.hpp>

template <typename T>
using option = boost::optional<T>;

#endif //OPTION_H
