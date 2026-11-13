// Original UG2 PC v1.2 0x7409e0, 122 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0073d820();
int __cdecl FUN_0073d8c0();
int __cdecl FUN_0073d8e0();
int __cdecl FUN_0073e5c0();
int __cdecl FUN_00740240();


void FUN_007409e0(undefined4 *param_1)

{
  int iVar1;
  char *pcVar2;
  
  if ((param_1 != (undefined4 *)0x0) && (param_1[0x31] != 0)) {
    iVar1 = FUN_0073d8c0(param_1[0x31]);
    while (iVar1 = iVar1 + -1, -1 < iVar1) {
      pcVar2 = (char *)FUN_0073d8e0(param_1[0x31],iVar1);
      if (pcVar2 != (char *)0x0) {
        if (param_1[3] != 5) {
          FUN_00740240(param_1,pcVar2,(undefined *)0x100000,*(uint *)(pcVar2 + 0x24),0,0);
        }
        FUN_0073d820(param_1[0x31],iVar1);
        (*(code *)param_1[1])(pcVar2);
      }
    }
    FUN_0073e5c0((int)param_1);
  }
  return;
}


