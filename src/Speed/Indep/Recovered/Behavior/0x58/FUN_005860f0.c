// Original UG2 PC v1.2 0x5860f0, 46 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008693c8 (*(undefined4 *)0x008693c8)
#define DAT_008651ac (*(undefined4 *)0x008651ac)

void FUN_005860f0(int param_1)

{
  undefined4 *puVar1;
  
  puVar1 = &DAT_008693c8;
  do {
    if (puVar1[1] == param_1) {
      if (puVar1 != (undefined4 *)0x0) {
        puVar1[7] = DAT_008651ac;
      }
      return;
    }
    puVar1 = puVar1 + 8;
  } while ((int)puVar1 < 0x869468);
  return;
}


