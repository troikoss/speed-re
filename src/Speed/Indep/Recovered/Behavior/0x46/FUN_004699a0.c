// Original UG2 PC v1.2 0x4699a0, 110 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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

#define DAT_0082b884 (*(undefined4 *)0x0082b884)

void FUN_004699a0(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  if (*(int *)(param_1 + 0x14) == 0) {
    return;
  }
  iVar2 = *(int *)(DAT_0082b884 + 0xb0);
  iVar3 = 0;
  do {
    if (iVar2 != 0) {
      for (iVar1 = *(int *)(iVar2 + 0x10); iVar1 != 0; iVar1 = *(int *)(iVar1 + 4)) {
        if (*(int *)(iVar1 + 0x10) == iVar3) {
          if ((iVar1 != 0) && (*(int *)(iVar1 + 0x30) == *(int *)(param_1 + 0x14))) {
            if (iVar2 == 0) {
              return;
            }
            iVar2 = *(int *)(iVar2 + 0x10);
            if (iVar2 == 0) {
              return;
            }
            do {
              if (*(int *)(iVar2 + 0x10) == iVar3) {
                return;
              }
              iVar2 = *(int *)(iVar2 + 4);
            } while (iVar2 != 0);
            return;
          }
          break;
        }
      }
    }
    iVar3 = iVar3 + 1;
    if (9 < iVar3) {
      return;
    }
  } while( true );
}


