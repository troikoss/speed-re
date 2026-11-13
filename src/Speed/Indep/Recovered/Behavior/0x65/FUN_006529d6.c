// Original UG2 PC v1.2 0x6529d6, 95 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_0075dd28();


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __stdcall FUN_006529d6(uint *param_1, int *param_2, int _pad0)

{
  int iVar1;
  uint uVar2;
  longlong lVar3;
  
  iVar1 = *param_2;
  if (iVar1 == 0) {
    uVar2 = (uint)(param_2[2] != 0);
  }
  else if ((iVar1 == 1) || (iVar1 == 2)) {
    uVar2 = param_2[2];
  }
  else {
    if (iVar1 != 3) {
      return 0x80004005;
    }
    lVar3 = FUN_0075dd28();
    uVar2 = (uint)lVar3;
  }
  *param_1 = uVar2;
  return 0;
}


