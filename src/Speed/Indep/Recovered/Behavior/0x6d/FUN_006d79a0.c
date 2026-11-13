// Original UG2 PC v1.2 0x6d79a0, 122 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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

#define DAT_008a7110 (*(undefined4 *)0x008a7110)
#define PTR_FUN_0080bfdc (*(code *)0x0080bfdc)

void __fastcall FUN_006d79a0(int *param_1)

{
  int *piVar1;
  
  piVar1 = (int *)*param_1;
  if (piVar1 != (int *)0x0) {
    if ((int *)piVar1[1] == (int *)0x0) {
      DAT_008a7110 = *piVar1;
    }
    else {
      *(int *)piVar1[1] = *piVar1;
    }
    if (*piVar1 != 0) {
      *(int *)(*piVar1 + 4) = piVar1[1];
    }
    if (piVar1[0x108] != 0) {
      (*(code *)PTR_FUN_0080bfdc)(piVar1[0x108],piVar1[4] << 2);
    }
    if (piVar1[0x109] != 0) {
      (*(code *)PTR_FUN_0080bfdc)(piVar1[0x109],piVar1[4]);
    }
    (*(code *)PTR_FUN_0080bfdc)(piVar1,0x430);
    *param_1 = 0;
  }
  *(undefined1 *)(param_1 + 0xb) = 0;
  return;
}


