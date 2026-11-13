// Original UG2 PC v1.2 0x459e00, 57 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_0082c6d4 (*(undefined4 *)0x0082c6d4)
#define DAT_0082c6d8 (*(undefined4 *)0x0082c6d8)
#define DAT_008a329c (*(undefined4 *)0x008a329c)
#define DAT_0088ec68 (*(undefined4 *)0x0088ec68)

undefined1 FUN_00459e00(uint param_1)

{
  undefined1 uVar1;
  
  uVar1 = 0;
  if (((param_1 == 0) || ((DAT_0082c6d4 < param_1 && (param_1 < DAT_0082c6d8 + DAT_0082c6d4)))) &&
     ((DAT_008a329c != 0 || (DAT_0088ec68 != 0)))) {
    uVar1 = 1;
  }
  return uVar1;
}


