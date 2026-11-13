// Original UG2 PC v1.2 0x7203eb, 36 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



undefined4 FUN_007203eb(byte *param_1)

{
  byte bVar1;
  int iVar2;
  
  bVar1 = *param_1;
  iVar2 = 0;
  if (bVar1 != 0) {
    do {
      param_1 = param_1 + 4;
      if (*(int *)param_1 != 0) {
        return 1;
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < (int)(uint)bVar1);
  }
  return 0;
}


