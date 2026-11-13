// Original UG2 PC v1.2 0x50ffb0, 89 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



int FUN_0050ffb0(undefined4 param_1,int param_2)

{
  switch(param_1) {
  case 3:
  case 7:
    return (-(uint)(param_2 != 1) & 0x5638af2) + 0x5e1f61ee;
  default:
    return 0;
  case 10:
    return (-(uint)(param_2 != 1) & 0xb9ad1f4c) + 0xcbbc8bf8;
  case 0x11:
  case 0x17:
  case 0x18:
    return (-(uint)(param_2 != 1) & 0x513709e0) + 0x5b81d613;
  }
}


