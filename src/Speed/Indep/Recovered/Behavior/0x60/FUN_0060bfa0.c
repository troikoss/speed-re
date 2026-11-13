// Original UG2 PC v1.2 0x60bfa0, 80 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00403d60();
int __cdecl FUN_00403d80();
int __cdecl FUN_00403da0();
int __cdecl FUN_0050d510();
#define DAT_007d7d3c (*(float *)0x007d7d3c)

float10 __fastcall FUN_0060bfa0(int param_1,int _edx_unused,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = FUN_00403d60(param_2);
  if ((char)uVar1 != '\0') {
    iVar2 = FUN_0050d510();
    return (float10)*(float *)(param_1 + 0x14 + iVar2 * 4);
  }
  uVar1 = FUN_00403d80(param_2);
  if ((char)uVar1 == '\0') {
    return (float10)DAT_007d7d3c;
  }
  iVar2 = FUN_00403da0(param_2);
  return (float10)*(float *)(param_1 + 0x18 + iVar2 * 4);
}


