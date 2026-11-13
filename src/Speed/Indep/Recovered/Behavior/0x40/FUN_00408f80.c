// Original UG2 PC v1.2 0x408f80, 34 bytes. BEHAVIOUR-VERIFIED ONLY (32 clean trials, 8 both-fault):
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

#define _DAT_00784360 (*(undefined4 *)0x00784360)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_00408f80(int param_1,int _edx_unused,int param_2)

{
  float fVar1;
  int iVar2;
  
  iVar2 = (int)*(char *)(param_2 + 8 + param_1);
  fVar1 = _DAT_00784360;
  if (iVar2 != -1) {
    fVar1 = *(float *)((iVar2 + -0x30) * 0x10 + param_1);
  }
  return (float10)fVar1;
}


