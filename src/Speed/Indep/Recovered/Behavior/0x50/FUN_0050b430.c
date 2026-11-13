// Original UG2 PC v1.2 0x50b430, 40 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.


typedef unsigned char undefined, undefined1, byte, uchar, bool8;
typedef unsigned short undefined2, ushort, word;
typedef unsigned int undefined4, uint, dword;
typedef unsigned __int64 undefined8, ulonglong;
typedef __int64 longlong;
typedef unsigned char bool;
#define true 1
#define false 0
typedef void (*code)();
typedef double float10;
typedef unsigned short wchar_t16;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;

#define DAT_00838490 (*(float *)0x00838490)
#define DAT_00838488 (*(float *)0x00838488)
#define DAT_00838494 (*(float *)0x00838494)
#define DAT_0083848c (*(float *)0x0083848c)

void FUN_0050b430(void)

{
  if (DAT_00838490 <= DAT_00838488) {
    DAT_00838490 = DAT_00838488;
  }
  DAT_00838494 = 0;
  DAT_0083848c = 0;
  DAT_00838488 = 0;
  return;
}


