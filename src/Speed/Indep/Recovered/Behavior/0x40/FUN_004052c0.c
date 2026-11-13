// Original UG2 PC v1.2 0x4052c0, 81 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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

#define DAT_0088a334 (*(undefined4 *)0x0088a334)

int __fastcall FUN_004052c0(void *_this,int _edx_unused,int param_1)

{
  short sVar1;
  short sVar2;
  int iVar3;
  
  iVar3 = 0;
  if ((((-1 < param_1) && (param_1 < *(char *)_this)) &&
      (sVar1 = *(short *)((int)_this + param_1 * 2 + 0x150), sVar1 != -1)) &&
     (((sVar2 = *(short *)((int)_this + param_1 * 2 + 0x178), sVar2 != -1 && (-1 < sVar1)) &&
      ((&DAT_0088a334)[sVar1] != 0)))) {
    iVar3 = sVar2 * 0x38 + 0x8c + (&DAT_0088a334)[sVar1];
  }
  return iVar3;
}


