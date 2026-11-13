// Original UG2 PC v1.2 0x722ac0, 135 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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
int __cdecl FUN_007214d2();
int __cdecl FUN_00721f99();
int __cdecl FUN_00721fa7();
int __cdecl FUN_00722172();
int __cdecl FUN_00722424();
int __cdecl FUN_007226a9();
int __cdecl FUN_007227cb();
int __cdecl FUN_0072366a();
int __cdecl FUN_00770dee();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b4668 (*(undefined4 *)0x008b4668)
#define DAT_008b4068 (*(undefined4 *)0x008b4068)

undefined4 FUN_00722ac0(int param_1)

{
  char cVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  
  if (DAT_008b4608 == '\0') {
    return 0xfffffff6;
  }
  FUN_00721f99();
  iVar2 = FUN_00722424(param_1);
  if (iVar2 == 0) {
    FUN_00721fa7();
    uVar4 = 0xfffffff8;
  }
  else {
    FUN_007227cb(param_1);
    FUN_00722172(iVar2);
    iVar3 = FUN_007226a9();
    if (iVar3 == 1) {
      FUN_007214d2(0x7229ee);
      DAT_008b4668 = 0;
    }
    FUN_0072366a(*(int *)(iVar2 + 8));
    cVar1 = *(char *)(iVar2 + 0x11);
    (&DAT_008b4068)[param_1] = 0;
    FUN_00721fa7();
    if (cVar1 == '\0') {
      FUN_00770dee();
    }
    uVar4 = 0;
  }
  return uVar4;
}


