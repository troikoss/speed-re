// Original UG2 PC v1.2 0x69a7ad, 59 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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



undefined4 __fastcall FUN_0069a7ad(void *_this,int _edx_unused,int param_1)

{
  byte *pbVar1;
  
  if (param_1 != -1) {
    do {
      pbVar1 = *(byte **)(*(int *)((int)_this + 0x14) + param_1 * 4);
      if (((*pbVar1 & 0x20) != 0) ||
         ((*(int *)(pbVar1 + 0x38) != -1 &&
          ((**(byte **)(*(int *)((int)_this + 0x14) + *(int *)(pbVar1 + 0x38) * 4) & 0x20) != 0)))) {
        return 1;
      }
      param_1 = *(int *)(pbVar1 + 8);
    } while (param_1 != -1);
  }
  return 0;
}


