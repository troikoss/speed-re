// Original UG2 PC v1.2 0x4abf70, 73 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __fastcall FUN_004abf70(int param_1)

{
  undefined4 uVar1;
  
  switch(*(undefined1 *)(param_1 + 0xf)) {
  case 0:
  case 3:
    uVar1 = 1;
    break;
  case 1:
    uVar1 = 4;
    break;
  default:
    uVar1 = 5;
    break;
  case 4:
    uVar1 = 2;
    break;
  case 5:
    uVar1 = 3;
  }
  if (*(char *)(param_1 + 0x34) == '\x02') {
    return 7;
  }
  if (*(char *)(param_1 + 0xd) == '\x01') {
    uVar1 = 8;
  }
  return uVar1;
}


