#include "NativeEngineBankIO.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundBanks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// The test needs only this POSIX entry point, not the Linux stat ABI headers.
extern "C" int mkdir(const char *, mode_t);
#include <unistd.h>
#include <string>

static int calls, failed;
struct Completion { char *buffer; const char *expected; int length, ordinal; };
static void __cdecl Complete(void *data) {
    Completion *value = static_cast<Completion *>(data);
    if (calls != value->ordinal || memcmp(value->buffer, value->expected, value->length) != 0 ||
        value->buffer[value->length] != 0x55) failed = 1;
    ++calls;
}
int main() {
    char root[] = "/tmp/ug2-bank-io-XXXXXX";
    if (!mkdtemp(root)) return 1;
    const std::string sound = std::string(root) + "/SOUND", engine = sound + "/ENGINE", file = engine + "/Test.GIN";
    if (mkdir(sound.c_str(), 0700) || mkdir(engine.c_str(), 0700)) return 2;
    FILE *out = fopen(file.c_str(), "wb");
    if (!out || fwrite("0123456789", 1, 10, out) != 10) return 3;
    fclose(out);
    NativeEngineBankIOBegin(root);
    char path[] = "sound\\engine\\test.gin";
    if (EngineBankFileSize57D900(path) != 10) return 4;
    char first[8], second[8]; memset(first, 0x55, sizeof(first)); memset(second, 0x55, sizeof(second));
    Completion a = {first, "2345", 4, 0}, b = {second, "012", 3, 1};
    EngineBankRead57B4A0(first, path, 2, 4, Complete, &a);
    EngineBankRead57B4A0(second, path, 0, 3, Complete, &b);
    memset(path, 'X', sizeof(path) - 1); // pending jobs must own their filenames
    if (calls || first[0] != 0x55 || second[0] != 0x55) return 5;
    char error[256]; int size = -1, secondSize = -1;
    if (!NativeEngineBankIODrain(error, sizeof(error), &size, &secondSize) || size != 4 || secondSize != 3 || calls != 2 || failed) return 6;
    if (EngineBankFileSize57D900("sound\\engine\\missing.gin") != -1) return 7;
    EngineBankRead57B4A0(0, "sound\\engine\\test.gin", 0, 10, Complete, &a);
    if (NativeEngineBankIODrain(error, sizeof(error)) || calls != 2) return 8;
    EngineBankRead57B4A0(first, "sound\\engine\\test.gin", 9, 4, Complete, &a);
    if (NativeEngineBankIODrain(error, sizeof(error)) || calls != 2) return 9;
    unlink(file.c_str()); rmdir(engine.c_str()); rmdir(sound.c_str()); rmdir(root);
    puts("bank I/O: case resolution, copied paths, deferred ordered callbacks, byte ranges and errors passed");
    return 0;
}
