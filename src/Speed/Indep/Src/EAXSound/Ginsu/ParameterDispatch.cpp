#include "EngineSound.h"

// UG2 PC v1.2 0x735580..0x7355c4. The OS results are ignored. The head is
// read after waiting, each next link after its callback, and the mutex again
// before release; callbacks may change these fields.
int EngineParameterDispatcher::Dispatch735580(void *values) {
    EngineDispatchWait(engineSoundMutex8B4F38, 0xffffffffu);
    EngineParameterCallback *node = first8;
    while (node != 0) {
        node->callback8(values, node->dataC);
        node = node->next0;
    }
    EngineDispatchRelease(engineSoundMutex8B4F38);
    return 0;
}
