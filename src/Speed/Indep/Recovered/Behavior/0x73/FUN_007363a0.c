// Original UG2 PC v1.2 0x7363a0, 81 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
int __cdecl FUN_00736350();


int FUN_007363a0(short *param_1)

{
  int iVar1;
  ushort *puVar2;
  int iVar3;
  int *local_4;
  
  iVar3 = 0;
  iVar1 = FUN_00736350((int)param_1,&local_4);
  if (iVar1 == 0) {
    return 0;
  }
  iVar1 = *local_4;
  if (*(ushort *)(iVar1 + 0x10) != 0) {
    puVar2 = (ushort *)(iVar1 + 0x18);
    do {
      if (*(short *)(iVar1 + (uint)*puVar2 * 4) == *param_1) {
        return iVar1 + (uint)*puVar2 * 4;
      }
      iVar3 = iVar3 + 1;
      puVar2 = puVar2 + 1;
    } while (iVar3 < (int)(uint)*(ushort *)(iVar1 + 0x10));
  }
  return 0;
}


