// Original UG2 PC v1.2 0x749440, 273 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_007555c0();
int __cdecl FUN_00755e80();
int __cdecl FUN_00755f50();


void FUN_00749440(int param_1)

{
  char cVar1;
  uint uVar2;
  undefined1 *puVar3;
  int iVar4;
  int iVar5;
  
  if (*(int *)(param_1 + 0x538) == 1) {
    FUN_007555c0(*(undefined4 **)(param_1 + 0x524));
    uVar2 = FUN_00755e80(*(int *)(param_1 + 0x524),0x64617461);
    if ((int)uVar2 < 0) {
      *(undefined4 *)(param_1 + 0x538) = 4;
    }
    else if (0 < (int)uVar2) {
      uVar2 = FUN_00755e80(*(int *)(param_1 + 0x524),0x636f6465);
      *(uint *)(param_1 + 0x538) = (-(uint)(uVar2 != 200) & 2) + 2;
    }
  }
  if (*(int *)(param_1 + 0x538) != 2) {
    return;
  }
  FUN_007555c0(*(undefined4 **)(param_1 + 0x524));
  uVar2 = FUN_00755f50(*(undefined4 **)(param_1 + 0x524),*(undefined4 **)(param_1 + 0x528),0,
                       *(uint *)(param_1 + 0x52c));
  if (uVar2 == 0xffffffff) {
    *(undefined4 *)(param_1 + 0x538) = 3;
    return;
  }
  if (-1 < (int)uVar2) {
    if ((int)uVar2 < 1) {
      return;
    }
    iVar4 = 0;
    iVar5 = 0;
    if (0 < (int)uVar2) {
      do {
        cVar1 = *(char *)(*(int *)(param_1 + 0x528) + iVar5);
        if (cVar1 != '\r') {
          *(char *)(*(int *)(param_1 + 0x528) + iVar4) = cVar1;
          iVar4 = iVar4 + 1;
        }
        iVar5 = iVar5 + 1;
      } while (iVar5 < (int)uVar2);
    }
    puVar3 = (undefined1 *)(*(int *)(param_1 + 0x528) + iVar4);
    *(int *)(param_1 + 0x52c) = *(int *)(param_1 + 0x52c) - iVar4;
    *(undefined1 **)(param_1 + 0x528) = puVar3;
    *puVar3 = 0;
    if (0 < *(int *)(param_1 + 0x52c)) {
      return;
    }
  }
  *(undefined4 *)(param_1 + 0x538) = 4;
  return;
}


