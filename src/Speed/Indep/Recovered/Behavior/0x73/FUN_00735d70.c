// Original UG2 PC v1.2 0x735d70, 87 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_007359e0();


int FUN_00735d70(int param_1,uint param_2)

{
  byte *pbVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  
  uVar4 = (uint)*(byte *)(param_1 + 6);
  if (uVar4 != 0) {
    pbVar1 = (byte *)FUN_007359e0(param_1);
    uVar2 = (uint)*pbVar1;
    iVar3 = 0;
    if (uVar4 != 0) {
      while( true ) {
        uVar2 = uVar2 - 1;
        if ((int)uVar2 < 0) {
          uVar2 = uVar4 - 1;
        }
        if (pbVar1[uVar2 + 1] == param_2) break;
        iVar3 = iVar3 + 1;
        if ((int)uVar4 <= iVar3) {
          return 0;
        }
      }
      return iVar3 + 1;
    }
  }
  return 0;
}


