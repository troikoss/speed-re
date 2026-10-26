#ifndef UG2_NATIVE_TEXTURE_UPLOAD_H
#define UG2_NATIVE_TEXTURE_UPLOAD_H
#include <stddef.h>
// CPU staging backend for the original texture API calls. No GL shading policy.
void NativeInitialiseTextureUploads();
void NativeDrainTextureUploads();
size_t NativeTextureBufferCount();
bool NativeTextureBufferExists(unsigned int token);
#endif
