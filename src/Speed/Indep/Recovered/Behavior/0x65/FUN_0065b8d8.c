// Original UG2 PC v1.2 0x65b8d8, 85 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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

#define DAT_007b7400 (*(undefined4 *)0x007b7400)
#define DAT_007d9904 (*(undefined4 *)0x007d9904)

undefined4 __stdcall FUN_0065b8d8(int *param_1,int *param_2,undefined4 *param_3)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  int *piVar4;
  bool bVar5;
  
  uVar2 = 0;
  if (param_3 == (undefined4 *)0x0) {
    uVar2 = 0x80004005;
  }
  else {
    iVar1 = 4;
    bVar5 = true;
    piVar3 = param_2;
    piVar4 = &DAT_007b7400;
    do {
      if (iVar1 == 0) break;
      iVar1 = iVar1 + -1;
      bVar5 = *piVar3 == *piVar4;
      piVar3 = piVar3 + 1;
      piVar4 = piVar4 + 1;
    } while (bVar5);
    if (!bVar5) {
      iVar1 = 4;
      bVar5 = true;
      piVar3 = &DAT_007d9904;
      do {
        if (iVar1 == 0) break;
        iVar1 = iVar1 + -1;
        bVar5 = *param_2 == *piVar3;
        param_2 = param_2 + 1;
        piVar3 = piVar3 + 1;
      } while (bVar5);
      if (!bVar5) {
        return 0x80004002;
      }
    }
    *param_3 = param_1;
    (**(code **)(*param_1 + 4))(param_1);
  }
  return uVar2;
}


