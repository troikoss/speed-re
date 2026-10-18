#include "FEngHash.h"

// Original 0x505450: null returns -1; bytes outside ASCII lowercase are unchanged.
unsigned int FngHash(const char *name) {
    unsigned int hash = 0xffffffff;
    if (name) {
        unsigned char c = (unsigned char)*name;
        while (c) {
            if (c > '`' && c < '{') c -= 0x20;
            hash = hash * 33 + c;
            ++name;
            c = (unsigned char)*name;
        }
    }
    return hash;
}
