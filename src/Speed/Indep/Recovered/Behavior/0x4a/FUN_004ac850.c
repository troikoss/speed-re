// Original UG2 PC v1.2 0x4ac850, 46 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_00537c00();
#define DAT_008651ac (*(undefined4 *)0x008651ac)
#define DAT_008364e0 (*(undefined4 *)0x008364e0)

void __fastcall FUN_004ac850(int param_1)

{
  *(undefined1 *)(param_1 + 0x54) = 1;
  *(undefined4 *)(param_1 + 0x4c) = DAT_008651ac;
  FUN_00537c00(*(byte **)(param_1 + 4),0x47ff4e7c,*(byte **)(param_1 + 0x58),'\x01');
  DAT_008364e0 = param_1;
  return;
}


