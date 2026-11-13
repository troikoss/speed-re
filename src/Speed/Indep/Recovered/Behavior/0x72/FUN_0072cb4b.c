// Original UG2 PC v1.2 0x72cb4b, 101 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_007328e8();
int __cdecl FUN_00732b10();
int __cdecl FUN_00732be1();
#define _DAT_008b4894 (*(undefined4 *)0x008b4894)
#define _DAT_008b48a0 (*(undefined4 *)0x008b48a0)
#define _DAT_008b4890 (*(undefined4 *)0x008b4890)
#define _DAT_008b489c (*(undefined4 *)0x008b489c)
#define _DAT_008b488c (*(undefined4 *)0x008b488c)
#define _DAT_008b4898 (*(undefined4 *)0x008b4898)
#define _DAT_008b482c (*(undefined4 *)0x008b482c)
#define _DAT_008b4834 (*(undefined4 *)0x008b4834)
#define _DAT_008b4830 (*(undefined4 *)0x008b4830)
#define _DAT_008b4838 (*(undefined4 *)0x008b4838)
#define _DAT_008b4840 (*(undefined4 *)0x008b4840)
#define _DAT_008b483c (*(undefined4 *)0x008b483c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0072cb4b(void)

{
  _DAT_008b4894 = 0x3c;
  _DAT_008b48a0 = 0x3c;
  _DAT_008b4890 = 0x30;
  _DAT_008b489c = 0x30;
  _DAT_008b488c = 0x2c;
  _DAT_008b4898 = 0x2c;
  _DAT_008b482c = FUN_00732be1;
  _DAT_008b4834 = FUN_00732b10;
  _DAT_008b4830 = FUN_007328e8;
  _DAT_008b4838 = FUN_00732be1;
  _DAT_008b4840 = FUN_00732b10;
  _DAT_008b483c = FUN_007328e8;
  return;
}


