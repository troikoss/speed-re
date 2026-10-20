#include "NativeEngineBankIO.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundBanks.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <string>
#include <vector>

namespace {
struct BankRead {
    void *destination;
    std::string filename;
    int offset, size;
    void (__cdecl *callback)(void *);
    void *clientdata;
};
std::string gameRoot;
std::vector<BankRead> pending;

// The reference uses Windows paths; local installation directories may differ
// in case on Linux. Resolve components without changing the recovered filename.
std::string LocalPath(const char *filename) {
    std::string name(filename);
    for (size_t i = 0; i < name.size(); ++i) if (name[i] == '\\') name[i] = '/';
    std::string path = gameRoot;
    size_t first = 0;
    while (first < name.size()) {
        const size_t slash = name.find('/', first);
        std::string part = name.substr(first, slash == std::string::npos ? slash : slash - first);
        DIR *directory = opendir(path.c_str());
        if (directory) {
            std::string match;
            while (dirent *entry = readdir(directory)) {
                if (part == entry->d_name) { match = part; break; }
                if (match.empty() && strcasecmp(part.c_str(), entry->d_name) == 0) match = entry->d_name;
            }
            closedir(directory);
            if (!match.empty()) part = match;
        }
        path += "/" + part;
        if (slash == std::string::npos) break;
        first = slash + 1;
    }
    return path;
}
}

void NativeEngineBankIOBegin(const char *directory) {
    gameRoot = directory;
    pending.clear();
}

int EngineBankFileSize57D900(const char *filename) {
    FILE *file = fopen(LocalPath(filename).c_str(), "rb");
    if (!file) return -1;
    const int seek = fseek(file, 0, SEEK_END);
    const long size = seek == 0 ? ftell(file) : -1;
    fclose(file);
    return size < 0 ? -1 : (int)size;
}

int EngineBankRead57B4A0(void *destination, const char *filename, int offset, int size,
                       void (__cdecl *callback)(void *), void *clientdata) {
    BankRead request = {destination, filename, offset, size, callback, clientdata};
    pending.push_back(request);
    return 0; // The recovered caller ignores the original request identifier.
}

bool NativeEngineBankIODrain(char *error, int errorSize, int *firstSize, int *secondSize) {
    if (firstSize) *firstSize = pending.empty() ? 0 : pending[0].size;
    if (secondSize) *secondSize = pending.size() < 2 ? 0 : pending[1].size;
    for (size_t i = 0; i < pending.size(); ++i) {
        const BankRead request = pending[i];
        FILE *file = request.destination && request.offset >= 0 && request.size >= 0
                         ? fopen(LocalPath(request.filename.c_str()).c_str(), "rb") : 0;
        bool ok = false;
        if (file) {
            ok = fseek(file, request.offset, SEEK_SET) == 0 &&
                 fread(request.destination, 1, request.size, file) == (size_t)request.size;
            fclose(file);
        }
        if (!ok) {
            snprintf(error, errorSize, "engine bank read failed: %s", request.filename.c_str());
            pending.clear();
            return false;
        }
        request.callback(request.clientdata);
    }
    pending.clear();
    return true;
}
