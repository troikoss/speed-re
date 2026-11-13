// Original UG2 PC v1.2 0x73afd0, 99 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0045a410();
int __cdecl FUN_0073b300();
int __cdecl FUN_0073b620();
#define DAT_008b7bbc (*(undefined4 *)0x008b7bbc)
#define DAT_008b7bc0 (*(undefined4 *)0x008b7bc0)
#define DAT_008b7bb0 (*(undefined4 *)0x008b7bb0)

void FUN_0073afd0(uint param_1,undefined4 param_2,undefined4 param_3)

{
  int extraout_EAX;
  int iVar1;
  
  if ((DAT_008b7bbc != '\0') && (DAT_008b7bc0 == 0)) {
    FUN_0073b620(-1);
    if (extraout_EAX != 0) {
      iVar1 = FUN_0073b300(param_1);
      if (iVar1 == 0) {
        iVar1 = -8;
      }
      else {
        iVar1 = *(int *)(iVar1 + 0x54);
      }
      FUN_0045a410(iVar1,param_2,param_3);
      DAT_008b7bc0 = 0;
      DAT_008b7bb0 = 0;
    }
  }
  return;
}


