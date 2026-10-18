#include "FEngResponse.h"
#include "FEngQueueNodes.h"

FEResponseCommand *__fastcall FEngConstructResponseCommand(FEResponseCommand *command) { // Original 0x4fe910
    command->type = 0;
    command->parameter = 0;
    command->target = 0;
    return command;
}

void __fastcall FEngDestructResponseCommand(FEResponseCommand *command) { // Original 0x517600
    unsigned int type = command->type;
    if (type >= 0x200 && type <= 0x204 && type != 0x203)
        FEngFreeQueueNode((void *)command->parameter);
    command->parameter = 0;
}

void FEResponseCommand::SetString(const char *text) { // Original 0x517630
    unsigned int value = type;
    if (value >= 0x200 && value <= 0x204 && value != 0x203)
        FEngFreeQueueNode((void *)parameter);
    parameter = 0;
    if (text) {
        unsigned int length = 0;
        do { ++length; } while (text[length - 1]);
        char *buffer = (char *)FEngAllocateQueueNode(length);
        char *destination = buffer;
        char c;
        do {
            c = *text++;
            *destination++ = c;
        } while (c);
        parameter = (unsigned int)buffer;
    }
}

FEResponseCommand *FEResponseCommand::Assign(const FEResponseCommand *source) { // Original 0x52ad80
    unsigned int value = type;
    if (value >= 0x200 && value <= 0x204 && value != 0x203)
        FEngFreeQueueNode((void *)parameter);
    parameter = 0;
    value = source->type;
    type = value;
    if (value >= 0x200 && value <= 0x204 && value != 0x203) {
        SetString((const char *)source->parameter);
        target = source->target;
        return this;
    }
    parameter = source->parameter;
    target = source->target;
    return this;
}
