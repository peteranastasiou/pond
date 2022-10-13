#pragma once

#include <string>

namespace util {

/**
 * Make a new std::string from printf style formatting
*/
std::string format(const char* format, ...);

}