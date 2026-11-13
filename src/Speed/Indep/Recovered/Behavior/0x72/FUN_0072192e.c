// Original UG2 PC v1.2 0x72192e, 32 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008b4050 (*(undefined4 *)0x008b4050)
#define DAT_008b3ff8 (*(undefined4 *)0x008b3ff8)

undefined4 FUN_0072192e(int param_1,undefined4 param_2,undefined4 param_3)

{
  if (param_1 == 0) {
    DAT_008b4050 = param_2;
    DAT_008b3ff8 = param_3;
    return 0;
  }
  return 0xfffffffb;
}


