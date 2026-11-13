// Original UG2 PC v1.2 0x50ef50, 96 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



int __stdcall FUN_0050ef50(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = 0;
  switch(param_1) {
  case 0x4d:
  case 0x4e:
  case 0x4f:
  case 0x50:
    return param_1 + (param_2 + -0x40) * 4;
  case 0x51:
  case 0x52:
  case 0x53:
  case 0x54:
    return param_1 + (param_2 + -0x41) * 4;
  case 0x55:
  case 0x56:
  case 0x57:
  case 0x58:
    return param_1 + (param_2 + -0x42) * 4;
  case 0x59:
  case 0x5a:
  case 0x5b:
  case 0x5c:
    iVar1 = 0x43;
  }
  return param_1 + (param_2 - iVar1) * 4;
}


