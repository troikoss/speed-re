#ifndef UG2_ORIGINAL_IMAGE_H
#define UG2_ORIGINAL_IMAGE_H

// Maps the player's own speed2.exe .rdata/.data at their original addresses so the
// recovered sources can bind original globals (see OriginalSymbols.ld). No original
// code is executed; .bss is zero-filled as at process start.
bool IsOriginalImageMapped(); // Native mapping ownership; no game behavior.
bool MapOriginalImage(const char *path, char *error, int errorSize);
// Native resource storage, retained from the same SHA-verified player image.
// An index is one-based; zero means no matching type/name or numeric identifier.
unsigned int FindOriginalResource(unsigned int type,const char *name,unsigned int identifier);
const unsigned char *OriginalResourceData(unsigned int index,unsigned int *size);

#endif
