// Original UG2 PC v1.2 0x5c0cd0, 74 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define DAT_008005c0 (*(undefined4 *)0x008005c0)
#define _DAT_00784250 (*(undefined4 *)0x00784250)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_005c0cd0(int param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  
  iVar3 = 0;
  if (DAT_008005c0 != 0) {
    puVar2 = &DAT_008005c0;
    iVar1 = DAT_008005c0;
    do {
      if ((iVar1 == param_1) && (puVar2[1] == param_2)) {
        return (float10)*(float *)(iVar3 * 0xc + 0x8005c8);
      }
      iVar1 = puVar2[3];
      puVar2 = puVar2 + 3;
      iVar3 = iVar3 + 1;
    } while (iVar1 != 0);
  }
  return (float10)_DAT_00784250;
}


