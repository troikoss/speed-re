// Original UG2 PC v1.2 0x69a3ef, 46 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



uint __stdcall FUN_0069a3ef(int param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  
  uVar2 = *(uint *)(*(int *)(*(int *)(param_3 + 0x14) + param_1 * 4) + 0x40);
  uVar1 = *(uint *)(*(int *)(*(int *)(param_3 + 0x14) + param_2 * 4) + 0x40);
  if (uVar2 < uVar1) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = (uint)(uVar1 < uVar2);
  }
  return uVar2;
}


