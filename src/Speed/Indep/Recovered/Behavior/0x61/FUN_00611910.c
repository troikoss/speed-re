// Original UG2 PC v1.2 0x611910, 60 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_0089f050 (*(undefined4 *)0x0089f050)
#define DAT_0089f218 (*(undefined4 *)0x0089f218)
#define DAT_0089f3e0 (*(undefined4 *)0x0089f3e0)
#define DAT_0089f5a8 (*(undefined4 *)0x0089f5a8)
#define DAT_0089f770 (*(undefined4 *)0x0089f770)
#define DAT_0089f938 (*(undefined4 *)0x0089f938)

undefined4 * FUN_00611910(undefined4 param_1)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)0x0;
  switch(param_1) {
  case 0x530b82b0:
    return &DAT_0089f050;
  case 0x530b82b1:
    return &DAT_0089f218;
  case 0x530b82b2:
    return &DAT_0089f3e0;
  case 0x530b82b3:
    return &DAT_0089f5a8;
  case 0x530b82b4:
    return &DAT_0089f770;
  case 0x530b82b5:
    puVar1 = &DAT_0089f938;
  }
  return puVar1;
}


