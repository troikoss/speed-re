#ifndef BWARE_FORMATTING_H
#define BWARE_FORMATTING_H

#ifdef _MSC_VER
typedef __int64 BFormatCounter;
#else
typedef long long BFormatCounter;
#endif

// Observed 12-byte formatter context; padding +5..+7 is not initialized by bVFormat.
struct BFormatContext {
    char *destination0;
    unsigned char value4;
    char unidentified5[3];
    unsigned int value8;
};
extern unsigned int bFormatShift8284E4;
extern float bFormatScale828028;
extern float bFormatTime828518;
extern unsigned int bFormatCount82851C;

int __stdcall bFormatQueryCounter(BFormatCounter *counter); // platform import at IAT 0x78311c
int bFormatCore(BFormatContext *context, const char *format, const void *arguments); // 0x43e950, remaining boundary
double bFormatElapsed(int start); // 0x43e5c0
int bVFormat(char *buffer, const char *format, const void *arguments); // 0x43fbc0

#endif
