// Original UG2 PC v1.2 0x65b098, 90 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



undefined4 __stdcall FUN_0065b098(int *param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  
  iVar3 = *param_1;
  iVar1 = *(int *)(iVar3 + 4);
  if (iVar1 < 0) {
LAB_0065b0e9:
    uVar4 = 0;
  }
  else {
    if (iVar1 < 2) {
      *param_1 = iVar3 + 0x1c;
    }
    else {
      if (((iVar1 < 4) || (iVar1 == 4)) || (iVar1 != 5)) goto LAB_0065b0e9;
      uVar2 = *(uint *)(iVar3 + 0x14);
      uVar5 = 0;
      *param_1 = iVar3 + 0x18;
      if (uVar2 != 0) {
        do {
          iVar3 = FUN_0065b098(param_1);
          if (iVar3 == 0) goto LAB_0065b0e9;
          uVar5 = uVar5 + 1;
        } while (uVar5 < uVar2);
      }
    }
    uVar4 = 1;
  }
  return uVar4;
}


