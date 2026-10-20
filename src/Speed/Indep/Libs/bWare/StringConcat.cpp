#include "Strings.h"

// 0x43dda0: concatenate without a capacity check, preserving the original
// byte read/write order. The caller supplies a 64-byte path buffer.
void bStringConcat43DDA0(char *destination, const char *directory, const char *filename) {
    int length = 0;
    while (*directory) destination[length++] = *directory++;
    while (*filename) destination[length++] = *filename++;
    destination[length] = 0;
}

