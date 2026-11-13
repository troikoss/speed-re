// Original UG2 PC v1.2 0x7367f0, 97 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00736700();
int __cdecl FUN_00737f40();
int __cdecl FUN_00738120();
int __cdecl FUN_007385b0();
#define DAT_008b4f50 (*(undefined4 *)0x008b4f50)
#define DAT_008b5028 (*(undefined4 *)0x008b5028)

void FUN_007367f0(uint param_1)

{
  int iVar1;
  int iVar2;
  
  if ((((DAT_008b4f50 == 0) && (DAT_008b4f50 = 1, param_1 < 8)) &&
      (iVar1 = *(int *)(&DAT_008b5028 + param_1 * 4), -1 < iVar1)) && (iVar1 < 0x10)) {
    FUN_00737f40(param_1);
    iVar2 = FUN_007385b0(iVar1);
    FUN_00736700(iVar1);
    if (iVar2 != 0) {
      FUN_00738120(param_1);
    }
  }
  DAT_008b4f50 = 0;
  return;
}


