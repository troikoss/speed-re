// Original UG2 PC v1.2 0x50e5a0, 149 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 __stdcall FUN_0050e5a0(int param_1)

{
  undefined4 uVar1;
  
  if ((0x60 < param_1) && (param_1 < 0x69)) {
    return 0x34;
  }
  if ((0x68 < param_1) && (param_1 < 0x71)) {
    return 0x35;
  }
  if ((0x70 < param_1) && (param_1 < 0x79)) {
    return 0x36;
  }
  if ((0x78 < param_1) && (param_1 < 0x81)) {
    return 0x37;
  }
  if ((0x80 < param_1) && (param_1 < 0x89)) {
    return 0x38;
  }
  if ((0x88 < param_1) && (param_1 < 0x91)) {
    return 0x39;
  }
  if ((param_1 < 0x91) || (uVar1 = 0x3a, 0x98 < param_1)) {
    uVar1 = 0xa9;
  }
  return uVar1;
}


