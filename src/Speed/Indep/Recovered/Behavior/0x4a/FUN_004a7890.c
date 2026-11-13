// Original UG2 PC v1.2 0x4a7890, 21 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00836750 (*(undefined4 *)0x00836750)
#define DAT_008371b0 (*(undefined4 *)0x008371b0)

undefined * FUN_004a7890(int param_1)

{
  undefined *puVar1;
  
  puVar1 = &DAT_00836750;
  if (param_1 != 1) {
    puVar1 = &DAT_008371b0;
  }
  return puVar1;
}


