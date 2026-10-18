#include "VehicleMover.h"

extern int replayState8669F4;
extern int replayState866A14;
extern unsigned char value850098;   // 0x850098
extern int replayValue864FAC;       // 0x864fac

// Original 0x5efe70.
bool ReplayRecorder::QueryState5EFE70() {
    return (value850098 || replayValue864FAC) && !flag119;
}

// Original 0x5f00f0. This recovers the predicate, not recording/playback.
bool ReplayRecorder::QueryState5F00F0() {
    return flag21 != 0 || replayState8669F4 > 0 || replayState866A14 > 0;
}
