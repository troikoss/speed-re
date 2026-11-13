// Original UG2 PC v1.2 0x72423e, 77 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined4 FUN_0072423e(undefined2 *param_1)

{
  undefined2 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  
  *(undefined1 *)(param_1 + 1) = 0;
  *param_1 = 0;
  *(undefined1 *)((int)param_1 + 3) = 0x7f;
  *(undefined1 *)(param_1 + 2) = 0x40;
  *(undefined1 *)((int)param_1 + 5) = 0;
  *(undefined1 *)(param_1 + 3) = 0;
  *(undefined1 *)((int)param_1 + 7) = 0;
  param_1[4] = 0;
  puVar2 = (undefined4 *)(param_1 + 0x2c);
  iVar3 = 4;
  do {
    puVar2[-4] = 0;
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  puVar2 = (undefined4 *)(param_1 + 0xc);
  puVar1 = param_1 + 6;
  iVar3 = 6;
  do {
    *puVar1 = 0;
    *puVar2 = 0;
    puVar1 = puVar1 + 1;
    puVar2 = puVar2 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return 0;
}


