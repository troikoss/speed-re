#ifndef UG2_NATIVE_ENGINE_BANK_IO_H
#define UG2_NATIVE_ENGINE_BANK_IO_H
// Demo file-I/O backend for the recovered bank loader. Requests copy their
// paths and remain pending until Drain; this is not the original stream scheduler.
void NativeEngineBankIOBegin(const char *gameDirectory);
bool NativeEngineBankIODrain(char *error, int errorSize, int *firstSize = 0);
#endif
