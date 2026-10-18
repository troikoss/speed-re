// Native clock/import and host formatter adapters. The original core 0x43e950 is pending.
#include "../../Speed/Indep/Libs/bWare/Formatting.h"
#include <SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
    if (context->value4) bFormatValue828150 = (unsigned char)context->value8;
    char *temporary = 0;
    int result = vasprintf(&temporary, format, arguments);
    va_end(arguments);
    if (result >= 0) {
        unsigned int count = 0;
        bFormatWrite(context, temporary, (unsigned int)result, &count);
        if (context->destination0) *context->destination0 = 0;
        free(temporary);
    }
    return result;
}
