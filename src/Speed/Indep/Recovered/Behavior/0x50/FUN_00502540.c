// Original UG2 PC v1.2 0x502540, 97 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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

#define DAT_0089e7d9 (*(undefined4 *)0x0089e7d9)
#define DAT_00890104 (*(undefined4 *)0x00890104)
#define DAT_0089e7a0 (*(undefined4 *)0x0089e7a0)

void __fastcall FUN_00502540(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)((int)_this + 0x10);
  iVar2 = 3;
  if (((DAT_0089e7d9 != '\0') || (DAT_00890104 != 0)) ||
     ((0x104d < DAT_0089e7a0 && (DAT_0089e7a0 < 0x1054)))) {
    iVar2 = 4;
  }
  if (param_1 == 1) {
    iVar1 = iVar1 + 1;
    if (iVar2 < iVar1) {
      iVar1 = 0;
    }
  }
  else if ((param_1 == -1) && (iVar1 = iVar1 + -1, iVar1 < 0)) {
    *(int *)((int)_this + 0x10) = iVar2;
    return;
  }
  *(int *)((int)_this + 0x10) = iVar1;
  return;
}


