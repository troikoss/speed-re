// Original UG2 PC v1.2 0x7331b8, 71 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_00727e2a();
int __cdecl FUN_0072c31e();
int __cdecl FUN_007330f5();


void FUN_007331b8(undefined4 *param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  
  *param_1 = FUN_007330f5;
  uVar1 = FUN_00727e0c(*(int *)(param_2 + 0x1c));
  iVar2 = FUN_0072c31e(uVar1);
  param_1[10] = iVar2;
  iVar2 = FUN_00727e2a(*(int *)(param_2 + 0x1c));
  param_1[8] = 0xffffffff;
  *(char *)((int)param_1 + 0x31) = (char)iVar2;
  param_1[7] = 0;
  param_1[9] = 0;
  *(undefined1 *)(param_1 + 0xc) = *(undefined1 *)(param_2 + 0x18);
  param_1[0xb] = 0;
  return;
}


