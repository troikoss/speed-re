#include "Formatting.h"

// Original 0x43cb40. The entry mode chooses count order; ring mode reloads mode each byte.
void bFormatWrite(BFormatContext *context, const char *source, unsigned int length,
                  unsigned int *count) {
    if (!context->value4) {
        *count += length;
        if (context->destination0) {
            while (length) {
                char value = *source;
                *context->destination0 = value;
                context->destination0 = context->destination0 + 1;
                ++source;
                --length;
            }
        }
    } else if (length) {
        unsigned int index = bFormatRingIndex828514;
        do {
            unsigned char mode = context->value4;
            char value = *source;
            if (mode) {
                bFormatRing828178[index] = (unsigned char)value;
                index = (index + 1) & 0x7f;
            } else if (context->destination0) {
                *context->destination0 = value;
                context->destination0 = context->destination0 + 1;
            }
            ++*count;
            ++source;
            --length;
        } while (length);
        bFormatRingIndex828514 = index;
    }
}
