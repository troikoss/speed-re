#include "Formatting.h"

// Original 0x43e5c0: shifted low-word difference wraps, then clamps on its sign bit.
double bFormatElapsed(int start) {
    BFormatCounter counter;
    bFormatQueryCounter(&counter);
    unsigned int shift = bFormatShift8284E4;
    unsigned char bits = (unsigned char)shift;
    BFormatCounter shifted = bits >= 64 ? (counter < 0 ? -1 : 0) : counter >> bits;
    unsigned int delta = (unsigned int)shifted - (unsigned int)start;
    if ((int)delta < 0) delta = 0;
    return (double)(int)(1u << (shift & 31)) * (int)delta * bFormatScale828028;
}

// Original 0x43fbc0: formatting result survives elapsed-time and profiling writes.
int bVFormat(char *buffer, const char *format, const void *arguments) {
    BFormatCounter counter;
    bFormatQueryCounter(&counter);
    unsigned char bits = (unsigned char)bFormatShift8284E4;
    BFormatCounter shifted = bits >= 64 ? (counter < 0 ? -1 : 0) : counter >> bits;
    int start = (int)shifted;
    BFormatContext context;
    context.destination0 = buffer;
    context.value4 = 0;
    context.value8 = 0;
    int result = bFormatCore(&context, format, arguments);
    double elapsed = bFormatElapsed(start);
    bFormatTime828518 = (float)(elapsed + bFormatTime828518);
    ++bFormatCount82851C;
    return result;
}
