// Original UG2 PC v1.2 0x70b59e, 61 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



uint FUN_0070b59e(int param_1,uint param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar3 = (int)param_2 >> 0x1f;
  iVar1 = (uVar3 ^ param_2) - uVar3;
  uVar4 = (uint)(iVar1 < param_1 * 2) * iVar1 - param_1;
  uVar2 = (int)uVar4 >> 0x1f;
  return (uVar2 - (uVar2 ^ uVar4)) + uVar3 + param_1 ^ uVar3;
}


