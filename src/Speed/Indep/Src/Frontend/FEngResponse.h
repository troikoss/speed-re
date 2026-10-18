#ifndef UG2_FENG_RESPONSE_H
#define UG2_FENG_RESPONSE_H

#include "../Hud/HudVisibility.h"

struct FEResponseCommand {
    unsigned int type;
    unsigned int parameter;
    unsigned int target;
    void SetString(const char *text); // 0x517630, owned parameter for string command types.
    FEResponseCommand *Assign(const FEResponseCommand *source); // 0x52ad80
};

// ECX-only array callback ABI; C++ member provenance is not established.
FEResponseCommand *__fastcall FEngConstructResponseCommand(FEResponseCommand *command); // 0x4fe910
void __fastcall FEngDestructResponseCommand(FEResponseCommand *command); // 0x517600

struct FEResponse {
    char unidentified0[4];
    FEResponse *next4;
    char unidentified8[4];
    unsigned int messageC;
    unsigned int commandCount10;
    const FEResponseCommand *commands14;
    unsigned int SkipConditionalBlock(unsigned int index); // 0x504490, thiscall
};
UG2_CHECK_SIZE(FEResponseCommand, 0xc);
UG2_CHECK_OFFSET(FEResponseCommand, parameter, 4);
UG2_CHECK_OFFSET(FEResponseCommand, target, 8);
UG2_CHECK_OFFSET(FEResponse, next4, 4);
UG2_CHECK_OFFSET(FEResponse, messageC, 0xc);
UG2_CHECK_OFFSET(FEResponse, commandCount10, 0x10);
UG2_CHECK_OFFSET(FEResponse, commands14, 0x14);

#endif
