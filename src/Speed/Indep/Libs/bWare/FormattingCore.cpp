#include "Formatting.h"
#include <stdarg.h>

extern signed char bFormatParserTable786CF0[91];
extern char *bFormatNegativeInf7EB3D8, *bFormatPositiveInf7EB3DC, *bFormatNaN7EB3E0;
extern char *bFormatNull7EB3E8, *bFormatBadPointer7EB3EC;
extern char bFormatDecimal7EB3F0, bFormatSeparator7EB3F1;
extern signed char bFormatGrouping7EB3F2;
extern const char bFormatBadVector786DF4[], bFormatVector2786E0C[], bFormatVector3786E1C[], bFormatInvalidVector786E30[];

// Original 0x43bd60: a failed read probe is accepted if the write probe succeeds.
int bFormatCheckPointer(const void *pointer) {
    if (bFormatBadReadPointer(pointer, 1) && bFormatBadWritePointer(pointer, 1)) return 0;
    return 1;
}

// Original 0x4400d0: recursive vector formatting retains the shared profiling boundary.
int bSFormat(char *buffer, const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    int result = bVFormat(buffer, format, arguments);
    va_end(arguments);
    return result;
}

// Original 0x43e950. The packed table supplies both character classes and parser transitions.
int bFormatCore(BFormatContext *context, const char *format, const void *arguments) {
    const unsigned char *args = (const unsigned char *)arguments;
    unsigned int count = 0, flags = 0, precision = 0, width = 0, prefixSize = 0;
    unsigned int size; // Original unsupported conversion-class paths retain an unset/prior size.
    int state = 0;
    char buffer[128], prefix[4];
    buffer[63] = 0;
    char *text = 0;
    if (context->value4) bFormatValue828150 = (unsigned char)context->value8;
    for (;;) {
        signed char c = *format;
        if (!c || (int)count < 0) break;
        ++format;
        int kind = c < 32 || c > 122 ? 0 : bFormatParserTable786CF0[c - 32] & 15;
        state = bFormatParserTable786CF0[state + kind * 8] >> 4;
        switch (state) {
        case 0:
            if (context->value4) {
                bFormatRing828178[bFormatRingIndex828514] = (unsigned char)c;
                bFormatRingIndex828514 = (bFormatRingIndex828514 + 1) & 127;
            } else if (context->destination0) {
                *context->destination0 = c;
                ++context->destination0;
            }
            ++count;
            break;
        case 1:
            text = buffer; precision = 0xffffffff; flags = 0; width = 0; prefixSize = 0;
            break;
        case 2:
            switch (c) {
            case '+': flags |= 1; break;
            case ' ': flags |= 2; break;
            case '-': flags |= 4; break;
            case '0': flags |= 8; break;
            case '#': flags |= 0x80; break;
            case '$': flags |= 0x2000; break;
            }
            break;
        case 3:
            if (c == '*') {
                width = *(const unsigned int *)args; args += 4;
                if ((int)width < 0) { flags |= 4; width = 0 - width; }
            } else width = width * 10 + c - '0';
            break;
        case 4: precision = 0; break;
        case 5:
            if (c == '*') {
                precision = *(const unsigned int *)args; args += 4;
                if ((int)precision < 0) precision = 0xffffffff;
            } else precision = precision * 10 + c - '0';
            break;
        case 6:
            if (c == 'h') flags |= 0x20;
            else if (c == 'l') flags |= 0x10;
            break;
        case 7: {
            unsigned int radix = 0, digitOffset = 0;
            if (c == 'c') { buffer[0] = *(const char *)args; args += 4; size = 1; }
            else if (c == 'n') {
                **(unsigned int * const *)args = count; args += 4; flags |= 0x1000;
            } else if (c == 's' || c == 'v') {
                char dimension = c == 'v' ? *format : 0;
                bool stringScan = true;
                if (c == 's') {
                    text = *(char * const *)args; args += 4;
                    if (!text) text = bFormatNull7EB3E8;
                    else if (!bFormatCheckPointer(text)) text = bFormatBadPointer7EB3EC;
                } else if (dimension == '2' || dimension == '3') {
                    const float *vector = *(const float * const *)args; args += 4;
                    if (!bFormatCheckPointer(vector)) text = (char *)bFormatBadVector786DF4;
                    else {
                        int result;
                        if (dimension == '2')
                            result = bSFormat(buffer + 64, bFormatVector2786E0C, width, precision, (double)vector[0], width, precision, (double)vector[1]);
                        else
                            result = bSFormat(buffer + 64, bFormatVector3786E1C, width, precision, (double)vector[0], width, precision, (double)vector[1], width, precision, (double)vector[2]);
                        unsigned int ignored = 0;
                        bFormatWrite(context, buffer + 64, (unsigned int)result + 1, &ignored);
                        if (context->destination0) --context->destination0;
                        if (context->value4 && (int)bFormatRingIndex828514 > 0) --bFormatRingIndex828514;
                        ++format;
                        flags |= 0x1000;
                        stringScan = false;
                    }
                } else text = (char *)bFormatInvalidVector786E30;
                if (stringScan) {
                    unsigned int remaining = (int)precision < 0 ? 0x7fffffff : precision;
                    char *p = text;
                    do { char value = *p++; if (!value) break; if (!remaining--) break; } while (true);
                    size = (unsigned int)(p - text - 1);
                }
            } else if (c == 'z') {
                unsigned int value = *(const unsigned int *)args; args += 4; flags |= 0x400;
                if (value >= 0x7fffffff) { value = 0 - value; flags |= 0x100; }
                unsigned int integer = value >> 16, fraction = value & 0xffff;
                if ((int)precision < 0) precision = 2;
                char *end = buffer + 62;
                text = end;
                if (fraction || precision) {
                    unsigned int requested = precision;
                    char *p = end + 1 - requested;
                    if ((int)precision > 0) {
                        unsigned int places = precision;
                        precision = 0;
                        do {
                            if (fraction < 0x10000) fraction *= 10;
                            unsigned int bound = 0, digit = 0;
                            do { bound += 0x10000; ++digit; } while (bound <= fraction);
                            --digit; *p++ = (char)(digit + '0'); fraction -= digit * 0x10000;
                        } while (--places);
                    }
                    if (!fraction && (int)precision > 0) {
                        unsigned int places = precision; precision = 0;
                        do { *p++ = '0'; } while (--places);
                    }
                    *p = 0; p -= 1 + requested; *p = '.'; text = p - 1;
                    if (fraction) {
                        if (fraction < 0x10000) fraction *= 10;
                        unsigned int bound = 0, digit = 0;
                        do { bound += 0x10000; ++digit; } while (bound <= fraction);
                        if (digit - 1 >= 5) {
                            char *q = end;
                            while (*q == '9') {
                                *q = '0';
                                if (q[-1] == '.') { ++integer; goto fixedInteger; }
                                --q;
                            }
                            ++*q;
                        }
                    }
                }
            fixedInteger:
                do { *text-- = (char)('0' + integer % 10); integer /= 10; } while (integer);
                size = (unsigned int)(end - text); ++text;
            } else if (c == 'f' || c == 'e' || c == 'E' || c == 'g' || c == 'G') {
                union { double value; unsigned int word[2]; } number;
                number.value = *(const double *)args; args += 8; flags |= 0x400;
                char exponentLetter = c == 'E' || c == 'G' ? 'E' : 'e';
                if (c != 'f') flags |= 0x800;
                if ((number.word[1] & 0x7ff00000) == 0x7ff00000 &&
                    (text = number.word[1] == 0xfff00000 ? bFormatNegativeInf7EB3D8 :
                            number.word[1] == 0x7ff00000 ? bFormatPositiveInf7EB3DC : bFormatNaN7EB3E0)) {
                    size = 7;
                } else {
                    long double value = number.value, factor = 1.0;
                    buffer[0] = '0'; text = buffer + 1; char *p = text;
                    if (value < 0.0) { flags |= 0x100; value = -value; }
                    if (precision == 0xffffffff) precision = 3;
                    int exponent = 0;
                    if ((flags & 0x800) && value != 0.0) {
                        if (value >= 1.0) do { ++exponent; value *= 0.1; } while (value >= 1.0);
                        do { --exponent; value *= 10.0; } while (value < 1.0);
                    } else {
                        while (factor < value) { ++exponent; factor *= 10.0; }
                        if (value < factor) { --exponent; factor *= 0.1; }
                    }
                    bool grouping = (flags & 0x2000) && bFormatGrouping7EB3F2 > 0;
                    char decimal = flags & 0x2000 ? bFormatDecimal7EB3F0 : '.';
                    int position = exponent + 1, initialPosition = position;
                    if (factor < 1.0) *p++ = '0';
                    else {
                        do {
                            if (grouping) {
                                if (!(position % bFormatGrouping7EB3F2) && position != initialPosition) *p++ = bFormatSeparator7EB3F1;
                                --position;
                            }
                            signed char digit = (signed char)(BFormatCounter)(value / factor);
                            *p++ = (char)(digit + '0'); value -= (long double)digit * factor; factor *= 0.1;
                        } while (factor >= 1.0);
                        if (p == buffer + 1) *p++ = '0';
                    }
                    if (precision || (flags & 0x80)) {
                        *p++ = decimal;
                        if (precision) {
                            unsigned int places = precision; precision = 0;
                            do {
                                value *= 10.0; signed char digit = (signed char)(BFormatCounter)value;
                                *p++ = (char)(digit + '0'); value -= (long double)digit;
                            } while (--places);
                        }
                    }
                    --precision;
                    if (10.0 * value >= 5.0) {
                        char *q = p - 1;
                        for (;;) {
                            while (*q == '9') *q-- = '0';
                            if (*q != decimal && (!grouping || *q != bFormatSeparator7EB3F1)) break;
                            --q;
                        }
                        ++*q;
                        if (q < buffer + 1) text = buffer;
                    }
                    if (flags & 0x800) {
                        *p++ = exponentLetter;
                        if (exponent < 0) { exponent = -exponent; *p++ = '-'; } else *p++ = '+';
                        signed char digit = (signed char)(exponent / 100);
                        *p++ = (char)(digit + '0'); exponent -= digit * 100;
                        digit = (signed char)(exponent / 10);
                        *p++ = (char)(digit + '0'); *p++ = (char)('0' + (exponent - digit * 10) % 10);
                        prefixSize = 0;
                    }
                    size = (unsigned int)(p - text);
                }
            } else {
                switch (c) {
                case 'd': case 'i': flags |= 0x400; radix = 10; break;
                case 'u': radix = 10; break;
                case 'b': radix = 2; break;
                case 'o': radix = 8; if (flags & 0x80) flags |= 0x200; break;
                case 'p': precision = 8; // fall through
                case 'X': radix = 16; digitOffset = 7; break;
                case 'x': radix = 16; digitOffset = 0x27; break;
                }
                if (radix) {
                    if (radix == 16 && (flags & 0x80)) { prefix[0] = '0'; prefix[1] = (char)(digitOffset + 0x51); prefixSize = 2; }
                    unsigned int value;
                    if (flags & 0x20) value = flags & 0x400 ? (unsigned int)*(const short *)args : *(const unsigned short *)args;
                    else value = *(const unsigned int *)args;
                    args += 4;
                    if ((flags & 0x400) && (int)value < 0) { value = 0 - value; flags |= 0x100; }
                    if ((int)precision < 0) precision = 1; else flags &= ~8u;
                    if (!value) prefixSize = 0;
                    char *end = buffer + 62; text = end;
                    int group = 0;
                    bool grouping = (flags & 0x2000) && bFormatGrouping7EB3F2 > 0;
                    while ((int)precision-- > 0 || value) {
                        if (grouping && ++group > bFormatGrouping7EB3F2) { *text-- = bFormatSeparator7EB3F1; group = 1; }
                        unsigned char digit = (unsigned char)(value % radix + '0');
                        if (digit > '9') digit += (unsigned char)digitOffset;
                        *text-- = (char)digit; value /= radix;
                    }
                    size = (unsigned int)(end - text);
                    if ((flags & 0x200) && (!size || text[1] != '0')) { *text-- = '0'; ++size; }
                    ++text;
                }
            }
            if (!(flags & 0x1000)) {
                if (flags & 0x400) {
                    if (flags & 0x100) { prefix[0] = '-'; prefixSize = 1; }
                    else if (flags & 1) { prefix[0] = '+'; prefixSize = 1; }
                    else if (flags & 2) { prefix[0] = ' '; prefixSize = 1; }
                }
                int padding = (int)(width - size - prefixSize);
                if (padding < 0) padding = 0;
                else if (padding && !(flags & 12)) {
                    unsigned int index = bFormatRingIndex828514;
                    do {
                        if (context->value4) { bFormatRing828178[index] = ' '; index = (index + 1) & 127; }
                        else if (context->destination0) { *context->destination0 = ' '; ++context->destination0; }
                        ++count;
                    } while (--padding);
                    bFormatRingIndex828514 = index; padding = -1;
                }
                if (prefixSize) bFormatWrite(context, prefix, prefixSize, &count);
                if (padding > 0 && !(flags & 4)) {
                    unsigned int index = bFormatRingIndex828514;
                    do {
                        if (context->value4) { bFormatRing828178[index] = '0'; index = (index + 1) & 127; }
                        else if (context->destination0) { *context->destination0 = '0'; ++context->destination0; }
                        ++count;
                    } while (--padding);
                    bFormatRingIndex828514 = index; padding = -1;
                }
                bFormatWrite(context, text, size, &count);
                if (padding > 0) {
                    unsigned int index = bFormatRingIndex828514;
                    do {
                        if (context->value4) { bFormatRing828178[index] = ' '; index = (index + 1) & 127; }
                        else if (context->destination0) { *context->destination0 = ' '; ++context->destination0; }
                        ++count;
                    } while (--padding);
                    bFormatRingIndex828514 = index;
                }
            }
            break;
        }
        }
    }
    if (context->destination0 && count != 0xffffffff) *context->destination0 = 0;
    return (int)count;
}
