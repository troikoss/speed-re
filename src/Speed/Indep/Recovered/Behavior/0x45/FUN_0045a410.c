// Original UG2 PC v1.2 0x45a410, 46 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
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

#define DAT_0082b620 (*(undefined4 *)0x0082b620)

void FUN_0045a410(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  if ((((-1 < param_1) && (param_1 < 0x18)) && (iVar1 = (&DAT_0082b620)[param_1], iVar1 != 0)) &&
     (*(char *)(iVar1 + 100) == '\0')) {
    *(undefined4 *)(iVar1 + 0x28) = param_2;
    *(undefined4 *)(iVar1 + 0x2c) = param_3;
  }
  return;
}


