// Original UG2 PC v1.2 0x6f45b0, 61 bytes. BEHAVIOUR-VERIFIED ONLY (39 clean trials, 1 both-fault):
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



int FUN_006f45b0(ushort param_1,int param_2,uint param_3,int param_4)

{
  int iVar1;
  uint uVar2;
  
  if (param_3 != 0) {
    do {
      iVar1 = (param_3 >> 1) * param_4;
      uVar2 = (uint)*(ushort *)(iVar1 + param_2);
      iVar1 = iVar1 + param_2;
      if (param_1 == uVar2) {
        return iVar1;
      }
      if (0 < (int)(param_1 - uVar2)) {
        param_2 = iVar1 + param_4;
        param_3 = param_3 - 1;
      }
      param_3 = param_3 >> 1;
    } while (param_3 != 0);
  }
  return 0;
}


