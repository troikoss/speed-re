#include "../../Libs/bWare/Formatting.h"
#include <stdarg.h>

// Original 0x50b9c0: format into a 256-byte local, then hash inline (no cihash call).
unsigned int FngFormatHash(const char *format, ...) {
    char text[256];
    va_list arguments;
    va_start(arguments, format);
    bVFormat(text, format, arguments);
    va_end(arguments);
    unsigned int hash = 0xffffffff;
    for (const unsigned char *p = (const unsigned char *)text; *p; ++p) {
        hash *= 33;
        unsigned char c = *p;
        if ((signed char)c >= 'a' && (signed char)c <= 'z') c += 'A' - 'a';
        hash += c;
    }
    return hash;
}

// Original 0x50c960: capacity is unused; propagate the formatter result.
int FngFormatBuffer(char *buffer, unsigned int capacity, const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    int result = bVFormat(buffer, format, arguments);
    va_end(arguments);
    return result;
}
