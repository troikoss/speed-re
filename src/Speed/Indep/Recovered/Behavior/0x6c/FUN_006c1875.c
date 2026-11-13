// Original UG2 PC v1.2 0x6c1875, 57 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



int __stdcall FUN_006c1875(uint param_1,uint param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = *(uint *)(*(int *)(*(int *)(param_3 + 0x10) + param_1 * 4) + 8);
  uVar2 = *(uint *)(*(int *)(*(int *)(param_3 + 0x10) + param_2 * 4) + 8);
  if (uVar2 <= uVar1) {
    if (uVar2 < uVar1) {
      return -1;
    }
    if (param_1 <= param_2) {
      return -(uint)(param_1 < param_2);
    }
  }
  return 1;
}


