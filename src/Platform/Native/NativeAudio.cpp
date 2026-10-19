// Platform scaffolding for the snd library entry points that the recovered Ginsu synthesizer submits
// packets to (retail 0x721f99 lock, 0x721fa7 unlock, 0x723515 submit). The native runtime has no sound
// device, so the synthesizer is not reached; reaching these stubs stops the program.
#include "NativeVehicle.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/Ginsu.h"

void GinsuAudioLock(void) { NativeTrap("snd lock 0x721f99"); }
void GinsuAudioUnlock(void) { NativeTrap("snd unlock 0x721fa7"); }
void GinsuAudioSubmit(unsigned int handle, void *packet) { NativeTrap("snd submit 0x723515"); }
