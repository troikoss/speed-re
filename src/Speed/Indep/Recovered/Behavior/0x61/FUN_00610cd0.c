// Original UG2 PC v1.2 0x610cd0, 86 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

#define DAT_00803698 (*(float *)0x00803698)
#define _DAT_00803940 (*(float *)0x00803940)
#define DAT_00803934 (*(float *)0x00803934)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00610cd0(int param_1)

{
  float *pfVar1;
  int *piVar2;
  float *pfVar3;
  
  if (*(int *)(param_1 + 4) != 0) {
    piVar2 = &DAT_00803698;
    pfVar3 = (float *)(param_1 + 0x20);
    do {
      if ((*piVar2 != 0) && (*pfVar3 <= _DAT_00803940)) {
        pfVar1 = (float *)&DAT_00803934;
        do {
          if (*pfVar3 < *pfVar1) break;
          pfVar1 = pfVar1 + 1;
        } while ((int)pfVar1 < 0x803940);
      }
      piVar2 = piVar2 + 4;
      pfVar3 = pfVar3 + 1;
    } while ((int)piVar2 < 0x8036f8);
  }
  return;
}


