// Original UG2 PC v1.2 0x720c5a, 34 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



undefined4 FUN_00720c5a(int param_1)

{
  undefined4 *puVar1;
  
  *(undefined1 *)(param_1 + 0x10) = 0xff;
  if (*(char *)(param_1 + 0xf) != '\0') {
    puVar1 = (undefined4 *)(param_1 + 0x1c + (uint)*(byte *)(param_1 + 0xe) * 0xc);
    puVar1[1] = 0;
    *puVar1 = 0;
  }
  return 0;
}


