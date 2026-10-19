// Driver for tools/verify_native_snd_helpers.py: runs the native SND_attrsetdef and SNDplaysetdef on the buffers
// given as hex lines on stdin ("attr play", 0x68 and 0x18 bytes) and prints the resulting buffers as hex lines.
#include "../src/Speed/Indep/Src/EAXSound/Ginsu/Ginsu.h"

#include <stdio.h>
#include <string.h>

static int HexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return c - 'A' + 10;
}

static bool ReadBytes(const char *text, unsigned char *dest, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (!text[2 * i] || !text[2 * i + 1]) return false;
        dest[i] = (unsigned char)(HexDigit(text[2 * i]) * 16 + HexDigit(text[2 * i + 1]));
    }
    return true;
}

static void PrintBytes(const unsigned char *bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) printf("%02x", bytes[i]);
}

int main() {
    static char line[1024];
    while (fgets(line, sizeof(line), stdin)) {
        unsigned char attr[0x68];
        unsigned char play[0x18];
        char attrText[2 * 0x68 + 1];
        char playText[2 * 0x18 + 1];
        if (sscanf(line, "%208s %48s", attrText, playText) != 2) return 1;
        if (!ReadBytes(attrText, attr, sizeof(attr)) || !ReadBytes(playText, play, sizeof(play))) return 1;
        SND_attrsetdef(reinterpret_cast<GinsuSndAttr *>(attr));
        SNDplaysetdef(reinterpret_cast<GinsuSndPlayOpts *>(play));
        PrintBytes(attr, sizeof(attr));
        printf(" ");
        PrintBytes(play, sizeof(play));
        printf("\n");
    }
    return 0;
}
