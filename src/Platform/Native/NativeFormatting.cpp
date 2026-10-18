// Native clock and page-permission imports for recovered bWare formatting.
#include "../../Speed/Indep/Libs/bWare/Formatting.h"
#include <SDL.h>
#include <stdio.h>

int __stdcall bFormatQueryCounter(BFormatCounter *counter) {
    static bool initialized = false;
    if (!initialized) {
        bFormatShift8284E4 = 0;
        bFormatScale828028 = (float)(1.0 / (double)SDL_GetPerformanceFrequency());
        initialized = true; // Native clock uses seconds; original initialization/units remain unverified.
    }
    *counter = (BFormatCounter)SDL_GetPerformanceCounter();
    return 1;
}

namespace {
// Linux page-permission adapter for the original one-byte Win32 probes.
int BadPointer(const void *pointer, unsigned int size, bool write) {
    if (!size) return 0;
    unsigned long long address = (unsigned long)pointer;
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps) return 1;
    char line[512], permissions[5];
    unsigned long long first, last;
    int bad = 1;
    while (fgets(line, sizeof(line), maps)) {
        if (sscanf(line, "%llx-%llx %4s", &first, &last, permissions) != 3) continue;
        if (address >= first && address + size <= last) {
            bad = permissions[write ? 1 : 0] != (write ? 'w' : 'r');
            break;
        }
    }
    fclose(maps);
    return bad;
}
}

int __stdcall bFormatBadReadPointer(const void *pointer, unsigned int size) {
    return BadPointer(pointer, size, false);
}
int __stdcall bFormatBadWritePointer(const void *pointer, unsigned int size) {
    return BadPointer(pointer, size, true);
}
