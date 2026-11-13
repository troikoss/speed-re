// Original UG2 PC v1.2 0x65b733, 90 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int bytes;

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


undefined4 __fastcall FUN_0065b733(undefined4 *param_1)

{
  int iVar1;
  uint bytes;
  void *pvVar2;
  undefined4 uVar3;
  
  iVar1 = (param_1[3] & 1) + (param_1[3] & 0xfffffffe);
  param_1[5] = iVar1;
  bytes = iVar1 + (uint)*(ushort *)(param_1 + 0xd) * 2;
  uVar3 = 0;
  param_1[3] = bytes;
  if ((uint)param_1[4] < bytes) {
    pvVar2 = FUN_0075c6c8(bytes);
    if (pvVar2 == (void *)0x0) {
      uVar3 = 0x8007000e;
    }
    else {
      if ((void *)*param_1 != (void *)0x0) {
        FUN_0075c6da((void *)*param_1);
        *param_1 = 0;
      }
      *param_1 = pvVar2;
      param_1[4] = param_1[3];
    }
  }
  return uVar3;
}


