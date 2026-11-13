// Original UG2 PC v1.2 0x571380, 121 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_004ff970();
#define DAT_00864f24 (*(undefined4 *)0x00864f24)
#define DAT_007a03d8 (*(undefined4 *)0x007a03d8)
#define DAT_007a026c (*(undefined4 *)0x007a026c)
#define DAT_0079c6f8 (*(undefined4 *)0x0079c6f8)
#define DAT_007a0258 (*(undefined4 *)0x007a0258)
#define DAT_007a024c (*(undefined4 *)0x007a024c)
#define DAT_007a0240 (*(undefined4 *)0x007a0240)
#define DAT_007a03c0 (*(undefined4 *)0x007a03c0)
#define DAT_007a03d0 (*(undefined4 *)0x007a03d0)
#define DAT_007a03b8 (*(undefined4 *)0x007a03b8)
#define DAT_007a03d4 (*(undefined4 *)0x007a03d4)
#define DAT_007a0298 (*(undefined4 *)0x007a0298)
#define DAT_007a0280 (*(undefined4 *)0x007a0280)
#define DAT_007a0224 (*(undefined4 *)0x007a0224)
#define DAT_007a0214 (*(undefined4 *)0x007a0214)

char * FUN_00571380(void)

{
  undefined4 uVar1;
  
  switch(DAT_00864f24) {
  case 0:
    return &DAT_007a03d8;
  case 1:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
    uVar1 = FUN_004ff970();
    switch(uVar1) {
    case 0:
      return &DAT_007a026c;
    case 1:
      return &DAT_0079c6f8;
    case 2:
      return &DAT_007a0258;
    case 3:
      return &DAT_007a024c;
    case 4:
      return &DAT_007a0240;
    case 5:
      return &DAT_007a03c0;
    case 6:
      return &DAT_007a03d0;
    case 7:
      return &DAT_007a03b8;
    }
    break;
  case 2:
  case 10:
    return &DAT_007a03d4;
  case 3:
    return &DAT_007a0298;
  case 4:
    return &DAT_007a0280;
  case 0xb:
    return &DAT_007a0224;
  case 0xc:
    return &DAT_007a0214;
  }
  return (char *)0x0;
}


