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
extern unsigned int bFormatRingIndex828514;
extern unsigned char bFormatRing828178[128];
extern unsigned char bFormatValue828150;

int __stdcall bFormatQueryCounter(BFormatCounter *counter); // platform import at IAT 0x78311c
int bFormatCore(BFormatContext *context, const char *format, const void *arguments); // 0x43e950
int __stdcall bFormatBadReadPointer(const void *pointer, unsigned int size); // IsBadReadPtr import
int __stdcall bFormatBadWritePointer(const void *pointer, unsigned int size); // IsBadWritePtr import
int bFormatCheckPointer(const void *pointer); // 0x43bd60
int bSFormat(char *buffer, const char *format, ...); // 0x4400d0
double bFormatElapsed(int start); // 0x43e5c0
int bVFormat(char *buffer, const char *format, const void *arguments); // 0x43fbc0
void bFormatWrite(BFormatContext *context, const char *source, unsigned int length,
                  unsigned int *count); // 0x43cb40

#endif
