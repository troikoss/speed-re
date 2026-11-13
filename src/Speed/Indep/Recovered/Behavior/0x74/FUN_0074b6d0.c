// Original UG2 PC v1.2 0x74b6d0, 87 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



int FUN_0074b6d0(byte *param_1,byte *param_2,int param_3)

{
  byte bVar1;
  int iVar2;
  
  if (param_3 < 0) {
    bVar1 = *param_1;
    while( true ) {
      if (bVar1 == 0) {
        return 0;
      }
      if (bVar1 != *param_2) break;
      bVar1 = param_1[1];
      param_1 = param_1 + 1;
      param_2 = param_2 + 1;
    }
    return (uint)*param_1 - (uint)*param_2;
  }
  if (0 < param_3) {
    iVar2 = (int)param_2 - (int)param_1;
    do {
      if ((uint)*param_1 - (uint)param_1[iVar2] != 0) {
        return (uint)*param_1 - (uint)param_1[iVar2];
      }
      param_1 = param_1 + 1;
      param_3 = param_3 + -1;
    } while (0 < param_3);
  }
  return 0;
}


