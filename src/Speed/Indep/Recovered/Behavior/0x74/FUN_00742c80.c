// Original UG2 PC v1.2 0x742c80, 289 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00742b60();
#define DAT_008b7c70 (*(undefined4 *)0x008b7c70)
#define DAT_008b7c60 (*(undefined4 *)0x008b7c60)
#define DAT_008b7c72 (*(undefined4 *)0x008b7c72)
#define DAT_008b7c71 (*(undefined4 *)0x008b7c71)
#define DAT_008b7c74 (*(undefined4 *)0x008b7c74)
#define DAT_008b7c61 (*(undefined4 *)0x008b7c61)
#define DAT_008b7c73 (*(undefined4 *)0x008b7c73)
#define DAT_008b7c76 (*(undefined4 *)0x008b7c76)
#define DAT_008b7c62 (*(undefined4 *)0x008b7c62)
#define DAT_008b7c75 (*(undefined4 *)0x008b7c75)
#define DAT_008b7c78 (*(undefined4 *)0x008b7c78)
#define DAT_008b7c63 (*(undefined4 *)0x008b7c63)
#define DAT_008b7c77 (*(undefined4 *)0x008b7c77)
#define DAT_008b7c7a (*(undefined4 *)0x008b7c7a)
#define DAT_008b7c64 (*(undefined4 *)0x008b7c64)
#define DAT_008b7c79 (*(undefined4 *)0x008b7c79)
#define DAT_008b7c7c (*(undefined4 *)0x008b7c7c)
#define DAT_008b7c65 (*(undefined4 *)0x008b7c65)
#define DAT_008b7c7b (*(undefined4 *)0x008b7c7b)
#define DAT_008b7c7d (*(undefined4 *)0x008b7c7d)
#define s_0123456789abcdef__08x_007d2858 ((char *)0x007d2858)

undefined1 * FUN_00742c80(void)

{
  int iVar1;
  
  if (DAT_008b7c70 == '\0') {
    iVar1 = FUN_00742b60(0x6d616378,(undefined4 *)&DAT_008b7c60);
    if (0 < iVar1) {
      DAT_008b7c72 = s_0123456789abcdef__08x_007d2858[DAT_008b7c60 & 0xf];
      DAT_008b7c71 = s_0123456789abcdef__08x_007d2858[DAT_008b7c60 >> 4];
      DAT_008b7c74 = s_0123456789abcdef__08x_007d2858[DAT_008b7c61 & 0xf];
      DAT_008b7c73 = s_0123456789abcdef__08x_007d2858[DAT_008b7c61 >> 4];
      DAT_008b7c76 = s_0123456789abcdef__08x_007d2858[DAT_008b7c62 & 0xf];
      DAT_008b7c75 = s_0123456789abcdef__08x_007d2858[DAT_008b7c62 >> 4];
      DAT_008b7c78 = s_0123456789abcdef__08x_007d2858[DAT_008b7c63 & 0xf];
      DAT_008b7c77 = s_0123456789abcdef__08x_007d2858[DAT_008b7c63 >> 4];
      DAT_008b7c7a = s_0123456789abcdef__08x_007d2858[DAT_008b7c64 & 0xf];
      DAT_008b7c79 = s_0123456789abcdef__08x_007d2858[DAT_008b7c64 >> 4];
      DAT_008b7c7c = s_0123456789abcdef__08x_007d2858[DAT_008b7c65 & 0xf];
      DAT_008b7c7b = s_0123456789abcdef__08x_007d2858[DAT_008b7c65 >> 4];
      DAT_008b7c70 = '$';
      DAT_008b7c7d = 0;
    }
  }
  return &DAT_008b7c70;
}


