#ifndef UG2_FENG_RESPONSE_ARRAY_H
#define UG2_FENG_RESPONSE_ARRAY_H
#include "FEngResponse.h"

// CRT iterator normal-return paths observed in this UG2 build. Compiler SEH
// prologues, exceptional rollback and unwind funclets are separate evidence.
typedef void *(__fastcall *FEngLifetimeConstructor)(void *);
typedef void (__fastcall *FEngLifetimeDestructor)(void *);
void __stdcall FEngConstructArrayWithCleanup(void *memory, unsigned int stride, unsigned int count,
                                            FEngLifetimeConstructor constructor,
                                            FEngLifetimeDestructor destructor); // 0x75c461
void __stdcall FEngDestructArrayWithCleanup(void *memory, unsigned int stride, unsigned int count,
                                           FEngLifetimeDestructor destructor); // 0x75c401
#endif
