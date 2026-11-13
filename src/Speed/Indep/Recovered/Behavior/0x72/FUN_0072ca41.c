// Original UG2 PC v1.2 0x72ca41, 134 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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
int __cdecl FUN_00727e0c();
int __cdecl FUN_0072a195();
int __cdecl FUN_0072c31e();
int __cdecl FUN_0072c941();
int __cdecl FUN_0072ca1b();


undefined4 FUN_0072ca41(undefined4 *param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  
  *param_1 = FUN_0072c941;
  param_1[1] = FUN_0072ca1b;
  if (param_3 < 0) {
    param_1[8] = 0xffffffff;
  }
  else {
    uVar1 = FUN_00727e0c(param_3);
    iVar2 = FUN_0072c31e(uVar1);
    param_1[8] = iVar2;
  }
  param_1[9] = 0x3f800000;
  param_1[7] = param_2 + 6;
  param_1[10] = 0;
  param_1[0xb] = (uint)*(byte *)(param_2 + 1) << 1;
  iVar2 = FUN_0072a195((byte *)(param_2 + 2),4);
  param_1[0xd] = 0;
  param_1[0xe] = 0;
  param_1[0xf] = 0;
  param_1[0x60a] = 0;
  param_1[0x60b] = 0;
  param_1[0xc] = iVar2;
  param_1[0x60c] = 0x10bc;
  return 0;
}


