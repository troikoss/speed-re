// Original UG2 PC v1.2 0x435880, 86 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00827db0 (*(undefined4 *)0x00827db0)
#define DAT_0089e7a4 (*(undefined4 *)0x0089e7a4)

void __stdcall FUN_00435880(int param_1, int param_2, int _pad0)

{
  uint *puVar1;
  undefined4 *puVar2;
  int iVar3;
  
  if (param_2 == DAT_00827db0) {
    for (puVar2 = *(undefined4 **)(param_1 + 0x14); puVar2 != (undefined4 *)(param_1 + 0x14);
        puVar2 = (undefined4 *)*puVar2) {
      iVar3 = puVar2[2];
      if (*(int *)(iVar3 + 0x14) != 0) {
        if (DAT_0089e7a4 == 1) {
          puVar1 = (uint *)(*(int *)(iVar3 + 0x14) + 0x34);
          *puVar1 = *puVar1 | 0x1000;
          *(undefined4 *)(*(int *)(iVar3 + 0x14) + 100) =
               *(undefined4 *)(*(int *)(iVar3 + 0x14) + 0x60);
        }
        *(undefined4 *)(*(int *)(iVar3 + 0x14) + 0x10) = 1;
      }
    }
  }
  return;
}


