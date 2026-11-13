// Original UG2 PC v1.2 0x401e80, 70 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_0082da5c (*(undefined4 *)0x0082da5c)
#define DAT_0082da60 (*(undefined4 *)0x0082da60)
#define DAT_0082da6c (*(undefined4 *)0x0082da6c)
#define DAT_0082da70 (*(undefined4 *)0x0082da70)

int FUN_00401e80(int param_1)

{
  uint uVar1;
  int iVar2;
  
  uVar1 = DAT_0082da5c + param_1 * 0x40;
  if (uVar1 < DAT_0082da60) {
    iVar2 = DAT_0082da5c;
    DAT_0082da5c = uVar1;
    return iVar2;
  }
  DAT_0082da6c = 1;
  DAT_0082da70 = DAT_0082da70 + param_1 * 0x40;
  return 0;
}


