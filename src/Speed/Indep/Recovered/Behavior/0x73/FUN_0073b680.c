// Original UG2 PC v1.2 0x73b680, 54 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0073b620();
#define DAT_008b5740 (*(undefined4 *)0x008b5740)

undefined4 FUN_0073b680(int param_1)

{
  int *piVar1;
  undefined4 extraout_EAX;
  int iVar2;
  
  iVar2 = 0;
  piVar1 = &DAT_008b5740;
  while ((piVar1[1] == 0 || (*piVar1 != param_1))) {
    piVar1 = piVar1 + 0x24a;
    iVar2 = iVar2 + 1;
    if (0x8b7bdf < (int)piVar1) {
      return 0;
    }
  }
  FUN_0073b620(iVar2);
  return extraout_EAX;
}


