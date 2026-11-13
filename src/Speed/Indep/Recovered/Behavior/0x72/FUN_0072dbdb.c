// Original UG2 PC v1.2 0x72dbdb, 56 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __fastcall FUN_0072dbdb(int param_1)

{
  int iVar1;
  
  if (*(char *)(param_1 + 0x18) == '\x01') {
    iVar1 = (uint)*(byte *)(param_1 + 0x48) * 0x300;
  }
  else {
    iVar1 = (uint)*(byte *)(param_1 + 0x48) * 0x900;
    *(int *)(param_1 + 0xc) = iVar1;
    if ((*(char *)(param_1 + 0x18) != '\x03') || (*(char *)(param_1 + 0x3b) == '\0'))
    goto LAB_0072dc08;
    iVar1 = iVar1 >> 1;
  }
  *(int *)(param_1 + 0xc) = iVar1;
LAB_0072dc08:
  *(undefined1 *)(param_1 + 0x38) = 1;
  *(undefined1 *)(param_1 + 0x39) = 1;
  return 0;
}


