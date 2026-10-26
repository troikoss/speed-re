#ifndef UG2_NATIVE_TEXTURE_RECORDS_H
#define UG2_NATIVE_TEXTURE_RECORDS_H
#include <stddef.h>
#include <string>
#include <vector>

// Platform file/stream lifetime, separate from recovered texture records.
// Synchronous preloading replaces stream scheduling; no GPU/material algorithm.
class NativeTextureRecords {
public:
    NativeTextureRecords() {}
    ~NativeTextureRecords();
    bool Add(const unsigned char *data, size_t bytes, const std::string &key);
    void Clear();
    size_t PackCount() const;
    size_t TextureCount() const;
private:
    struct File;
    NativeTextureRecords(const NativeTextureRecords &) = delete;
    NativeTextureRecords &operator=(const NativeTextureRecords &) = delete;
    std::vector<File *> files_;
};
#endif
