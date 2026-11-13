// Original UG2 PC v1.2 0x73f940, 71 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



bool FUN_0073f940(int *param_1)

{
  uint uVar1;
  bool bVar2;
  
  uVar1 = *(uint *)(*param_1 + 0xf8);
  if (((uVar1 >> 2 & 1) == 0) && ((uVar1 & 0x200) == 0)) {
    bVar2 = false;
  }
  else {
    bVar2 = true;
  }
  return !bVar2 && (uVar1 & 0xc0000) == 0;
}


