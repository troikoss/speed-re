// Original UG2 PC v1.2 0x754bd0, 99 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_00440540();
int __cdecl FUN_00754b10();


uint FUN_00754bd0(int *param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,
                 uint param_5)

{
  uint uVar1;
  uint uVar2;
  undefined4 *puVar3;
  
  uVar1 = FUN_00754b10(param_1,param_2,param_3,(undefined4 *)0x0);
  if (-1 < (int)uVar1) {
    if ((int)uVar1 < (int)param_5) {
      param_5 = uVar1;
    }
    if (param_4 != (undefined4 *)0x0) {
      puVar3 = (undefined4 *)(param_1[0x1f] + 0xc);
      for (uVar2 = param_5 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
        *param_4 = *puVar3;
        puVar3 = puVar3 + 1;
        param_4 = param_4 + 1;
      }
      for (uVar2 = param_5 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
        *(undefined1 *)param_4 = *(undefined1 *)puVar3;
        puVar3 = (undefined4 *)((int)puVar3 + 1);
        param_4 = (undefined4 *)((int)param_4 + 1);
      }
    }
    FUN_00440540((void *)param_1[0x1f]);
    param_1[0x1f] = 0;
  }
  return uVar1;
}


