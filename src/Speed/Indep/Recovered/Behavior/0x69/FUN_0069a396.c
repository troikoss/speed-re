// Original UG2 PC v1.2 0x69a396, 56 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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



uint __stdcall FUN_0069a396(int param_1,int param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  
  iVar1 = *(int *)(*(int *)(param_3 + 0xbc) + 0x58);
  uVar3 = *(uint *)(iVar1 + param_1 * 0xc);
  uVar2 = *(uint *)(iVar1 + param_2 * 0xc);
  if (uVar2 < uVar3) {
    uVar3 = 0xffffffff;
  }
  else {
    uVar3 = (uint)(uVar3 < uVar2);
  }
  return uVar3;
}


