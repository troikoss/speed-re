// Original UG2 PC v1.2 0x46c470, 131 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_00721f99();
int __cdecl FUN_00721fa7();
int __cdecl FUN_00722fd0();
int __cdecl FUN_00723bc1();
int __stdcall FUN_0046b670(int);
#define DAT_0082b620 (*(undefined4 *)0x0082b620)

int FUN_0046c470(int param_1,int param_2,int param_3)

{
  uint *puVar1;
  int iVar2;
  
  puVar1 = (uint *)(&DAT_0082b620)[param_1];
  iVar2 = -8;
  if (puVar1 != (uint *)0x0) {
    iVar2 = FUN_0046b670(*puVar1);
    if ((char)puVar1[0x19] != '\0') {
      FUN_00721f99();
      iVar2 = FUN_00723bc1(*puVar1,param_2,param_3);
      FUN_00721fa7();
      puVar1[4] = param_3;
      return iVar2;
    }
    FUN_00721f99();
    iVar2 = FUN_00722fd0(*(int *)(iVar2 + 0x18),param_2,param_3);
    FUN_00721fa7();
    puVar1[4] = param_3;
  }
  return iVar2;
}


