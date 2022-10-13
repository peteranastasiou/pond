
#include "util.hpp"
#include <stdarg.h>
#include <string.h>

namespace util {

std::string format(const char* fmt, ...) {
    va_list args;

    // guess at buffer size:
    size_t initLen = 8;
    char * buf = (char *) malloc(initLen);  // use malloc so we can reallocate

    // Have a crack with the buffer size & find out actual size
    va_start(args, fmt);
    size_t len = vsnprintf(buf, initLen, fmt, args);
    va_end(args);

    // Check if we got everything
    if( len >= initLen ){
        // Not big enough - resize: 
        buf = (char *) realloc(buf, len+1);

        // format to string again:
        va_start(args, fmt);
        vsnprintf(buf, len+1, fmt, args);
        va_end(args);
    }
    std::string s = std::string(buf);
    free(buf);
    return s;
}

}
