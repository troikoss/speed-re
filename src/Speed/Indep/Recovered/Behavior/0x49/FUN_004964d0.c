// Original UG2 PC v1.2 0x4964d0, 232 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008635a9 (*(undefined4 *)0x008635a9)
#define DAT_008635aa (*(undefined4 *)0x008635aa)
#define DAT_008635ab (*(undefined4 *)0x008635ab)
#define DAT_008635ac (*(undefined4 *)0x008635ac)
#define DAT_008635ad (*(undefined4 *)0x008635ad)
#define DAT_008635ae (*(undefined4 *)0x008635ae)
#define DAT_008635b0 (*(undefined4 *)0x008635b0)
#define DAT_008635b2 (*(undefined4 *)0x008635b2)
#define DAT_008635b3 (*(undefined4 *)0x008635b3)
#define DAT_008635b5 (*(undefined4 *)0x008635b5)
#define DAT_008635b6 (*(undefined4 *)0x008635b6)
#define DAT_008635b4 (*(undefined4 *)0x008635b4)
#define DAT_008635b7 (*(undefined4 *)0x008635b7)
#define DAT_008635b8 (*(undefined4 *)0x008635b8)

uint FUN_004964d0(uint param_1)

{
  switch(param_1) {
  case 0:
    return (uint)(DAT_008635a9 == '\0');
  case 1:
    return (uint)(DAT_008635aa == '\0');
  case 2:
    return (uint)(DAT_008635ab == '\0');
  case 3:
    return (uint)(DAT_008635ac == '\0');
  case 4:
    return (uint)(DAT_008635ad == '\0');
  case 5:
    return (uint)(DAT_008635ae == '\0');
  case 6:
    return (uint)(DAT_008635b0 == '\0');
  case 7:
    return (uint)(DAT_008635b2 == '\0');
  case 8:
    return (uint)(DAT_008635b3 == '\0');
  case 9:
    return (uint)(DAT_008635b5 == '\0');
  case 10:
    return (uint)(DAT_008635b6 == '\0');
  case 0xb:
    return (uint)(DAT_008635b4 == '\0');
  case 0xc:
    return (uint)(DAT_008635b7 == '\0');
  case 0xd:
    param_1 = (uint)(DAT_008635b8 == '\0');
  }
  return param_1;
}


