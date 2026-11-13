// Original UG2 PC v1.2 0x496d10, 59 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00505450();
#define s_Untitled_Race_0078f734 ((char *)0x0078f734)

uint FUN_00496d10(undefined4 param_1)

{
  uint uVar1;
  
  switch(param_1) {
  case 0:
    return 0x268d063c;
  case 1:
    return 0xba613729;
  case 2:
    return 0xfda6758;
  default:
    uVar1 = FUN_00505450(s_Untitled_Race_0078f734);
    return uVar1;
  case 4:
    return 0x40c91ba7;
  case 5:
    return 0x59ecb2c2;
  }
}


