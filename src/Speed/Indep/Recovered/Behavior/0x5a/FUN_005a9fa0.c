// Original UG2 PC v1.2 0x5a9fa0, 69 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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

#define DAT_0086b398 (*(undefined4 *)0x0086b398)
#define DAT_0086b394 (*(undefined4 *)0x0086b394)
#define DAT_0086b390 (*(undefined4 *)0x0086b390)

void __stdcall FUN_005a9fa0(undefined4 *param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar2 = DAT_0086b398;
  uVar1 = DAT_0086b394;
  *param_1 = DAT_0086b390;
  param_1[1] = uVar1;
  param_1[2] = uVar2;
  uVar2 = DAT_0086b398;
  uVar1 = DAT_0086b394;
  *param_2 = DAT_0086b390;
  param_2[1] = uVar1;
  param_2[2] = uVar2;
  return;
}


