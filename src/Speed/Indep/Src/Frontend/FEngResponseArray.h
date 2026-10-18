#ifndef UG2_FENG_RESPONSE_ARRAY_H
#define UG2_FENG_RESPONSE_ARRAY_H
#include "FEngResponse.h"

// CRT iterator normal-return paths observed in this UG2 build. Compiler SEH
// prologues, exceptional rollback and unwind funclets are separate evidence.
typedef FEResponseCommand *(__fastcall *FEngCommandConstructor)(FEResponseCommand *);
typedef void (__fastcall *FEngCommandDestructor)(FEResponseCommand *);
void __stdcall FEngConstructResponseCommands(void *memory, unsigned int stride, unsigned int count,
                                            FEngCommandConstructor constructor,
                                            FEngCommandDestructor destructor); // 0x75c461
void __stdcall FEngDestructResponseCommands(void *memory, unsigned int stride, unsigned int count,
                                           FEngCommandDestructor destructor); // 0x75c401
#endif
