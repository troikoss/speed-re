// Original UG2 PC v1.2 0x7444c0, 112 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int __cdecl FUN_0073d6d0();
int __cdecl FUN_0073da80();
int __cdecl FUN_007551a0();


undefined4 FUN_007444c0(int param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  
  uVar2 = 0;
  if ((-1 < param_2) && (param_2 < 0x24)) {
    iVar1 = param_1 + param_2 * 0x14;
    *(undefined4 *)(iVar1 + 0x1168) = param_3;
    *(undefined4 *)(param_1 + (param_2 * 5 + 0x45b) * 4) = param_4;
    puVar3 = FUN_007551a0(1000,2000);
    *(undefined4 **)(iVar1 + 0x1160) = puVar3;
    puVar3 = FUN_0073d6d0(100,100,0);
    *(undefined4 **)(iVar1 + 0x1164) = puVar3;
    FUN_0073da80((int)puVar3,*(undefined4 *)(iVar1 + 0x1160));
    uVar2 = *(undefined4 *)(iVar1 + 0x1164);
  }
  return uVar2;
}


