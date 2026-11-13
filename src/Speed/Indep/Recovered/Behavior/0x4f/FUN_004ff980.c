// Original UG2 PC v1.2 0x4ff980, 77 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



void FUN_004ff980(undefined1 *param_1,int param_2,short *param_3)

{
  short sVar1;
  uint uVar2;
  
  if ((param_1 != (undefined1 *)0x0) && (param_3 != (short *)0x0)) {
    sVar1 = *param_3;
    uVar2 = 0;
    if (sVar1 != 0) {
      do {
        if (param_2 - 1U <= uVar2) break;
        if ((-0x81 < sVar1) && (sVar1 < 0x80)) {
          *param_1 = (char)*param_3;
          param_1 = param_1 + 1;
          uVar2 = uVar2 + 1;
        }
        param_3 = param_3 + 1;
        sVar1 = *param_3;
      } while (sVar1 != 0);
    }
    *param_1 = 0;
  }
  return;
}


