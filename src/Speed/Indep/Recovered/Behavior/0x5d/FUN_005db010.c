// Original UG2 PC v1.2 0x5db010, 106 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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
int __cdecl FUN_00571070();
int __cdecl FUN_00571130();
int __cdecl FUN_0060f700();


void __fastcall FUN_005db010(int param_1)

{
  *(undefined4 *)(param_1 + 0x168) = 0;
  *(undefined1 *)(param_1 + 0x16e) = 0;
  FUN_0060f700();
  FUN_00571130(0);
  FUN_00571130(1);
  FUN_00571130(2);
  FUN_00571130(3);
  *(undefined4 *)(param_1 + 0xd8) = 0;
  *(undefined4 *)(param_1 + 0x134) = 0;
  if (*(int *)(param_1 + 0x6b4) != 0) {
    FUN_00571070(*(int *)(param_1 + 0x6b4),*(undefined4 *)(param_1 + 0x6b8));
    *(undefined4 *)(param_1 + 0x6b4) = 0;
    *(undefined4 *)(param_1 + 0x6b8) = 0;
  }
  return;
}


