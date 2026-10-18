// Native clock/import and host formatter adapters. The original core 0x43e950 is pending.
#include "../../Speed/Indep/Libs/bWare/Formatting.h"
#include <SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

int bFormatCore(BFormatContext *context, const char *format, const void *rawArguments) {
    // The recovered wrapper supplies a memory sink (value4 == 0), without a capacity.
    va_list arguments;
    static_assert(sizeof(arguments) == sizeof(rawArguments), "native formatter requires i386 va_list");
    memcpy(&arguments, &rawArguments, sizeof(arguments));
    int result = context->destination0 ? vsprintf(context->destination0, format, arguments)
                                       : vsnprintf(0, 0, format, arguments);
    va_end(arguments);
    if (context->destination0 && result >= 0) context->destination0 += result;
    return result;
}
