
#include "util.hpp"
#include <stdarg.h>
#include <string.h>

namespace util {

void format(const char* fmt, ...) {
    // guess at buffer size:
    size_t initLen = 32;
    char * buf = new char[initLen];

    va_list args;
    va_start(args, fmt);
    
    // Get actual length needed:
    size_t len = vsnprintf(buf, initLen, fmt, args);
    
    // Check if we got everything
    if( len >= initLen ){
        delete buf;
        buf = new char[len+1];
        vsnprintf(buf, len+1, fmt, args);
    }
    va_end(args);

    std::string s = std::string(buf);
    delete buf
    return s;
}

}
