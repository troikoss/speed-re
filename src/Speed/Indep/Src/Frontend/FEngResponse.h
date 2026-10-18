#ifndef UG2_FENG_RESPONSE_H
#define UG2_FENG_RESPONSE_H

#include "../Hud/HudVisibility.h"

struct FEResponseCommand {
    unsigned int type;
    unsigned int parameter;
    unsigned int target;
};

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
