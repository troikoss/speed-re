#include "FEngResponse.h"

// Original 0x504490. The loop can read commands14[commandCount10] when no
// matching delimiter is found; do not replace the original bound with count-1.
unsigned int FEResponse::SkipConditionalBlock(unsigned int index) {
    unsigned int count = commandCount10;
    if (index == count - 1) return count;
    int depth = 1;
    const FEResponseCommand *command = commands14 + index;
    do {
        unsigned int type = command[1].type;
        ++command;
        ++index;
        if (type <= 0x500) {
            if (type == 0x500) {
                if (depth == 1) depth = 0;
            } else if (type >= 0x300 && type <= 0x301) {
                ++depth;
            }
        } else if (type == 0x501) {
            --depth;
        }
    } while (index < count && depth != 0);
    return index;
}
