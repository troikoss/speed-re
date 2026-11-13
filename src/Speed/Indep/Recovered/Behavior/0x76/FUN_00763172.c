// Original UG2 PC v1.2 0x763172, 127 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int mem;

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
int __cdecl FUN_0075c6c8();
int __cdecl FUN_0075c6da();
int __cdecl FUN_0076315d();
int __cdecl FUN_007631f1();
int __cdecl FUN_007661ba();
int __cdecl FUN_0076c3b6();
#define DAT_0081a340 (*(undefined4 *)0x0081a340)

undefined4 FUN_00763172(int param_1)

{
  void *mem;
  int *piVar1;
  undefined4 uVar2;
  int extraout_EAX;
  
  piVar1 = &DAT_0081a340 + param_1 * 2;
  if (*piVar1 != 0) {
    return 1;
  }
  mem = FUN_0075c6c8(0x18);
  if (mem == (void *)0x0) {
LAB_00763199:
    piVar1 = FUN_007661ba();
    *piVar1 = 0xc;
    uVar2 = 0;
  }
  else {
    FUN_007631f1(10);
    if (*piVar1 == 0) {
      FUN_0076c3b6(mem,4000);
      if (extraout_EAX == 0) {
        FUN_0075c6da(mem);
        FUN_0076315d(10);
        goto LAB_00763199;
      }
      *piVar1 = (int)mem;
    }
    else {
      FUN_0075c6da(mem);
    }
    FUN_0076315d(10);
    uVar2 = 1;
  }
  return uVar2;
}


