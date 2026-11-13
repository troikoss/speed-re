// Original UG2 PC v1.2 0x7630bf, 73 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0076c3b6();
#define DAT_008b85c0 (*(undefined4 *)0x008b85c0)
#define DAT_0081a344 (*(undefined4 *)0x0081a344)
#define DAT_0081a340 (*(undefined4 *)0x0081a340)

/* Library Function - Single Match
    __mtinitlocks
   
   Library: Visual Studio 2003 Release */

int __cdecl FUN_007630bf(void)

{
  int extraout_EAX;
  int iVar1;
  undefined *puVar2;
  
  iVar1 = 0;
  puVar2 = &DAT_008b85c0;
  do {
    if ((&DAT_0081a344)[iVar1 * 2] == 1) {
      (&DAT_0081a340)[iVar1 * 2] = puVar2;
      puVar2 = puVar2 + 0x18;
      FUN_0076c3b6((&DAT_0081a340)[iVar1 * 2],4000);
      if (extraout_EAX == 0) {
        (&DAT_0081a340)[iVar1 * 2] = 0;
        return 0;
      }
    }
    iVar1 = iVar1 + 1;
  } while (iVar1 < 0x24);
  return 1;
}


