// Original UG2 PC v1.2 0x7226c9, 218 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00722424();
int __cdecl FUN_00727030();
int __cdecl FUN_00727992();
int __cdecl FUN_007279b7();
int __cdecl FUN_00770e12();
int __cdecl FUN_00770e1b();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)

undefined4 FUN_007226c9(uint param_1,undefined4 param_2,undefined4 param_3,int param_4,int param_5)

{
  uint *puVar1;
  undefined4 uVar2;
  int iVar3;
  int extraout_EAX;
  int extraout_EAX_00;
  undefined8 *extraout_EAX_01;
  undefined4 extraout_ECX;
  undefined4 extraout_EDX;
  
  if (DAT_008b4608 == '\0') {
    return 0xfffffff6;
  }
  iVar3 = FUN_00722424(param_1);
  if (iVar3 == 0) {
    return 0xfffffff8;
  }
  if (*(int *)(iVar3 + 0x158) == 0) {
    return 0xfffffff3;
  }
  if (param_5 == 0) {
    FUN_00770e12();
    param_4 = extraout_EAX;
  }
  else if (param_5 == 1) {
    FUN_00770e1b();
    param_4 = extraout_EAX_00;
  }
  if (param_4 == 0) {
    return 0xffffffff;
  }
  FUN_00721f99();
  FUN_007279b7((int *)(iVar3 + 0x150));
  FUN_00727030(extraout_ECX,extraout_EDX,extraout_EAX_01,0x28);
  FUN_00727992((undefined4 *)(iVar3 + 0x144),(undefined4 *)extraout_EAX_01);
  puVar1 = (uint *)(iVar3 + 0xc);
  *(int *)(extraout_EAX_01 + 1) = param_4;
  *puVar1 = *puVar1 + 0x100;
  if ((int)*puVar1 < 0) {
    *puVar1 = 0;
  }
  *(uint *)((int)extraout_EAX_01 + 0xc) = *puVar1 | param_1;
  *(undefined4 *)(extraout_EAX_01 + 4) = param_2;
  uVar2 = *(undefined4 *)((int)extraout_EAX_01 + 0xc);
  FUN_00721fa7();
  return uVar2;
}


