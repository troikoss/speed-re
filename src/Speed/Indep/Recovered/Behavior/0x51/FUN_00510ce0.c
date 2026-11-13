// Original UG2 PC v1.2 0x510ce0, 83 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_004022c0();
int __cdecl FUN_00577eb0();
int __cdecl FUN_00580e90();
#define DAT_0083857c (*(undefined4 *)0x0083857c)
#define DAT_00838580 (*(undefined4 *)0x00838580)

void __fastcall FUN_00510ce0(int param_1)

{
  int *piVar1;
  
  if (DAT_0083857c != (int *)0x0) {
    FUN_00580e90(DAT_0083857c);
    DAT_00838580 = 0;
    DAT_0083857c = (int *)0x0;
  }
  if (*(char *)(param_1 + 0x80) != '\0') {
    FUN_004022c0();
    piVar1 = (int *)FUN_00577eb0((char *)(param_1 + 0x81));
    FUN_00580e90(piVar1);
    *(undefined1 *)(param_1 + 0x80) = 0;
  }
  return;
}


