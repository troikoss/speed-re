// Original UG2 PC v1.2 0x4a9500, 143 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
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
int __cdecl FUN_0057e480();
int __cdecl FUN_0057e4d0();


void __stdcall FUN_004a9500(undefined4 *param_1)

{
  uint uVar1;
  
  FUN_0057e4d0((char *)*param_1,8);
  uVar1 = FUN_0057e480(param_1[1],0x10,8);
  param_1[1] = uVar1;
  uVar1 = FUN_0057e480(param_1[2],0x20,8);
  param_1[2] = uVar1;
  uVar1 = FUN_0057e480(param_1[3],0x20,8);
  param_1[3] = uVar1;
  uVar1 = FUN_0057e480(param_1[4],0x20,8);
  param_1[4] = uVar1;
  uVar1 = FUN_0057e480(param_1[6],0x20,8);
  param_1[6] = uVar1;
  uVar1 = FUN_0057e480(param_1[5],0x20,8);
  param_1[5] = uVar1;
  FUN_0057e4d0((char *)(param_1 + 7),8);
  FUN_0057e4d0((char *)((int)param_1 + 0x1e),8);
  return;
}


