// Original UG2 PC v1.2 0x723e1e, 82 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
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
int __cdecl FUN_00724cb0();
int __cdecl FUN_00725480();
int __cdecl FUN_00727fbd();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)

int FUN_00723e1e(uint param_1)

{
  uint uVar1;
  int iVar2;
  int local_8;
  
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  uVar1 = FUN_00725480(param_1);
  if (-1 < (int)uVar1) {
    local_8 = -1;
    iVar2 = FUN_00727fbd(uVar1,&local_8);
    while (iVar2 != 0) {
      FUN_00724cb0(local_8);
      iVar2 = FUN_00727fbd(uVar1,&local_8);
    }
  }
  return uVar1;
}


