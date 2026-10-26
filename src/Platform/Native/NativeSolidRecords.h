#ifndef UG2_NATIVE_SOLID_RECORDS_H
#define UG2_NATIVE_SOLID_RECORDS_H
#include <stddef.h>
#include <vector>
struct CarRenderSlotTable;

// Platform ownership for original, mutable pack records. GPU tokens currently
// own CPU upload storage; GPU texture/effect setup and draw consumption remain seams.
class NativeSolidRecords {
public:
    NativeSolidRecords() {}
    bool Load(const unsigned char *data, size_t bytes);
    ~NativeSolidRecords();
    CarRenderSlotTable *Find(unsigned int hash);
    void Clear(); // Drop borrowed records before changing their texture owner.
private:
    NativeSolidRecords(const NativeSolidRecords &) = delete;
    NativeSolidRecords &operator=(const NativeSolidRecords &) = delete;
    std::vector<unsigned char> storage_;
    std::vector<unsigned int *> roots_;
};
#endif
