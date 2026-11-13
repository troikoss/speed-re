// Original UG2 PC v1.2 0x6f4730, 98 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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
int __cdecl FUN_006f46e0();


byte __fastcall FUN_006f4730(uint *param_1)

{
  char cVar1;
  uint uVar2;
  
  uVar2 = *param_1 & 0xff;
  switch(uVar2) {
  case 0x69:
    return 0x80;
  case 0x6f:
    return 0x20;
  case 0x7c:
    return 0x10;
  case 0x81:
    return 1;
  }
  switch(uVar2) {
  case 0x29:
  case 0x2a:
  case 0x2d:
  case 0x2f:
  case 0x32:
    return 4;
  default:
    cVar1 = FUN_006f46e0(uVar2);
    return -(cVar1 != '\0') & 2;
  }
}


