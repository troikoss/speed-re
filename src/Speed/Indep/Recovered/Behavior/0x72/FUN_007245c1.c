// Original UG2 PC v1.2 0x7245c1, 94 bytes. BEHAVIOUR-VERIFIED ONLY (32 clean trials, 8 both-fault):
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

#define DAT_008b402c (*(undefined4 *)0x008b402c)
#define DAT_00818084 (*(undefined4 *)0x00818084)
#define DAT_008b4028 (*(undefined4 *)0x008b4028)
#define DAT_008b400c (*(undefined4 *)0x008b400c)
#define DAT_008b4008 (*(undefined4 *)0x008b4008)
#define DAT_008b4034 (*(undefined4 *)0x008b4034)
#define DAT_008b4030 (*(undefined4 *)0x008b4030)

undefined4 FUN_007245c1(int param_1)

{
  bool bVar1;
  
  if (DAT_008b402c == 1) {
    DAT_00818084 = DAT_008b4028;
    DAT_008b402c = 0;
  }
  bVar1 = false;
  while (!bVar1) {
    if (DAT_008b400c != 0) {
      *(undefined1 *)(param_1 + 0x2a) = DAT_008b4008;
      DAT_008b400c = 0;
    }
    if (DAT_008b4034 != 0) {
      *(undefined2 *)(param_1 + 0x26) = DAT_008b4030;
      DAT_008b4034 = 0;
    }
    bVar1 = true;
  }
  return 0xfffffffb;
}


