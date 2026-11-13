// Original UG2 PC v1.2 0x77d010, 541 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_007f6bb0 (*(undefined4 *)0x007f6bb0)
#define DAT_0078e024 (*(undefined4 *)0x0078e024)
#define _DAT_007f6bb4 (*(undefined4 *)0x007f6bb4)
#define _DAT_007f6bb8 (*(undefined4 *)0x007f6bb8)
#define _DAT_007f6bbc (*(undefined4 *)0x007f6bbc)
#define _DAT_007f6bc0 (*(undefined4 *)0x007f6bc0)
#define _DAT_007f6bc4 (*(undefined4 *)0x007f6bc4)
#define _DAT_007f6bc8 (*(undefined4 *)0x007f6bc8)
#define _DAT_007f6bcc (*(undefined4 *)0x007f6bcc)
#define _DAT_007f6bd0 (*(undefined4 *)0x007f6bd0)
#define _DAT_007f6bd4 (*(undefined4 *)0x007f6bd4)
#define _DAT_007f6bd8 (*(undefined4 *)0x007f6bd8)
#define _DAT_007f6bdc (*(undefined4 *)0x007f6bdc)
#define _DAT_007f6be0 (*(undefined4 *)0x007f6be0)
#define _DAT_007f6be4 (*(undefined4 *)0x007f6be4)
#define _DAT_007f6be8 (*(undefined4 *)0x007f6be8)
#define _DAT_007f6bec (*(undefined4 *)0x007f6bec)
#define _DAT_007f6bf0 (*(undefined4 *)0x007f6bf0)
#define _DAT_007f6bf4 (*(undefined4 *)0x007f6bf4)
#define _DAT_007f6bf8 (*(undefined4 *)0x007f6bf8)
#define _DAT_007f6bfc (*(undefined4 *)0x007f6bfc)
#define _DAT_007f6c00 (*(undefined4 *)0x007f6c00)
#define _DAT_007f6c04 (*(undefined4 *)0x007f6c04)
#define _DAT_007f6c08 (*(undefined4 *)0x007f6c08)
#define _DAT_007f6c0c (*(undefined4 *)0x007f6c0c)
#define _DAT_007f6c10 (*(undefined4 *)0x007f6c10)
#define _DAT_007f6c14 (*(undefined4 *)0x007f6c14)
#define _DAT_007f6c18 (*(undefined4 *)0x007f6c18)
#define _DAT_007f6c1c (*(undefined4 *)0x007f6c1c)
#define _DAT_007f6c20 (*(undefined4 *)0x007f6c20)
#define _DAT_007f6c24 (*(undefined4 *)0x007f6c24)
#define _DAT_007f6c28 (*(undefined4 *)0x007f6c28)
#define _DAT_007f6c2c (*(undefined4 *)0x007f6c2c)
#define _DAT_007f6c30 (*(undefined4 *)0x007f6c30)
#define _DAT_007f6c34 (*(undefined4 *)0x007f6c34)
#define _DAT_007f6c38 (*(undefined4 *)0x007f6c38)
#define _DAT_007f6c3c (*(undefined4 *)0x007f6c3c)
#define _DAT_007f6c40 (*(undefined4 *)0x007f6c40)
#define _DAT_007f6c44 (*(undefined4 *)0x007f6c44)
#define _DAT_007f6c48 (*(undefined4 *)0x007f6c48)
#define _DAT_007f6c4c (*(undefined4 *)0x007f6c4c)
#define _DAT_007f6c50 (*(undefined4 *)0x007f6c50)
#define _DAT_007f6c54 (*(undefined4 *)0x007f6c54)
#define _DAT_007f6c58 (*(undefined4 *)0x007f6c58)
#define _DAT_007f6c5c (*(undefined4 *)0x007f6c5c)
#define _DAT_007f6c60 (*(undefined4 *)0x007f6c60)
#define _DAT_007f6c64 (*(undefined4 *)0x007f6c64)
#define _DAT_007f6c68 (*(undefined4 *)0x007f6c68)
#define _DAT_007f6c6c (*(undefined4 *)0x007f6c6c)
#define _DAT_007f6c70 (*(undefined4 *)0x007f6c70)
#define _DAT_007f6c74 (*(undefined4 *)0x007f6c74)
#define _DAT_007f6c78 (*(undefined4 *)0x007f6c78)
#define _DAT_007f6c7c (*(undefined4 *)0x007f6c7c)
#define _DAT_007f6c80 (*(undefined4 *)0x007f6c80)
#define _DAT_007f6c84 (*(undefined4 *)0x007f6c84)
#define _DAT_007f6c88 (*(undefined4 *)0x007f6c88)
#define _DAT_007f6c8c (*(undefined4 *)0x007f6c8c)
#define _DAT_007f6c90 (*(undefined4 *)0x007f6c90)
#define _DAT_007f6c94 (*(undefined4 *)0x007f6c94)
#define _DAT_007f6c98 (*(undefined4 *)0x007f6c98)
#define _DAT_007f6c9c (*(undefined4 *)0x007f6c9c)
#define _DAT_007f6ca0 (*(undefined4 *)0x007f6ca0)
#define _DAT_007f6ca4 (*(undefined4 *)0x007f6ca4)
#define _DAT_007f6ca8 (*(undefined4 *)0x007f6ca8)
#define _DAT_007f6cac (*(undefined4 *)0x007f6cac)
#define _DAT_007f6cb0 (*(undefined4 *)0x007f6cb0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077d010(void)

{
  DAT_007f6bb0 = DAT_0078e024;
  _DAT_007f6bb4 = 0x3e800000;
  _DAT_007f6bb8 = 0xbdcccccd;
  _DAT_007f6bbc = 0x41200000;
  _DAT_007f6bc0 = 1;
  _DAT_007f6bc4 = 1;
  _DAT_007f6bc8 = 0x3f000000;
  _DAT_007f6bcc = 0x40100000;
  _DAT_007f6bd0 = 0;
  _DAT_007f6bd4 = 0x3ecccccd;
  _DAT_007f6bd8 = 0;
  _DAT_007f6bdc = 0x3f800000;
  _DAT_007f6be0 = 0x40533333;
  _DAT_007f6be4 = 0x42200000;
  _DAT_007f6be8 = 0;
  _DAT_007f6bec = 0;
  _DAT_007f6bf0 = 0x3f800000;
  _DAT_007f6bf4 = 0x3ecccccd;
  _DAT_007f6bf8 = 0;
  _DAT_007f6bfc = 0xbf800000;
  _DAT_007f6c00 = 1;
  _DAT_007f6c04 = 0x3f000000;
  _DAT_007f6c08 = 0x40100000;
  _DAT_007f6c0c = 0x41880000;
  _DAT_007f6c10 = 0x3ecccccd;
  _DAT_007f6c14 = 0;
  _DAT_007f6c18 = 0x3f800000;
  _DAT_007f6c1c = 0x40000000;
  _DAT_007f6c20 = 0;
  _DAT_007f6c24 = 0;
  _DAT_007f6c28 = 0;
  _DAT_007f6c2c = 0x3f800000;
  _DAT_007f6c30 = 0x3ecccccd;
  _DAT_007f6c34 = 0;
  _DAT_007f6c38 = 0xbf800000;
  _DAT_007f6c3c = 0;
  _DAT_007f6c40 = 0x3f000000;
  _DAT_007f6c44 = 0x40100000;
  _DAT_007f6c48 = 0;
  _DAT_007f6c4c = 0x3ecccccd;
  _DAT_007f6c50 = 0;
  _DAT_007f6c54 = 0x3f800000;
  _DAT_007f6c58 = 0x4007df3b;
  _DAT_007f6c5c = 0x42200000;
  _DAT_007f6c60 = 0;
  _DAT_007f6c64 = 0;
  _DAT_007f6c68 = 0xbf800000;
  _DAT_007f6c6c = 0x3ecccccd;
  _DAT_007f6c70 = 0;
  _DAT_007f6c74 = 0xbf800000;
  _DAT_007f6c78 = 0;
  _DAT_007f6c7c = 0x3f000000;
  _DAT_007f6c80 = 0x40100000;
  _DAT_007f6c84 = 0x41880000;
  _DAT_007f6c88 = 0x3ecccccd;
  _DAT_007f6c8c = 0;
  _DAT_007f6c90 = 0x3f800000;
  _DAT_007f6c94 = 0x40000000;
  _DAT_007f6c98 = 0;
  _DAT_007f6c9c = 0;
  _DAT_007f6ca0 = 0;
  _DAT_007f6ca4 = 0xbf800000;
  _DAT_007f6ca8 = 0x3ecccccd;
  _DAT_007f6cac = 0;
  _DAT_007f6cb0 = 0xbf800000;
  return;
}


