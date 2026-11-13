// Original UG2 PC v1.2 0x5e0260, 76 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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
int __cdecl FUN_005dbdd0();


undefined4 * __fastcall FUN_005e0260(undefined4 *param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 *puVar4;
  
  param_1[0xeb] = 0;
  puVar2 = param_1 + 0xe2;
  puVar1 = param_1 + 0x1d;
  iVar3 = 9;
  puVar4 = param_1;
  do {
    puVar4 = puVar4 + 3;
    *puVar4 = 0;
    puVar1[-1] = 0;
    *puVar1 = 0;
    puVar1[1] = 0;
    puVar1[2] = 0;
    puVar1[3] = 0;
    puVar1[4] = 0;
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
    puVar1 = puVar1 + 0x16;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  FUN_005dbdd0();
  return param_1;
}


