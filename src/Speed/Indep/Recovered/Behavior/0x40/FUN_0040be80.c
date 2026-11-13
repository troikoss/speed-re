// Original UG2 PC v1.2 0x40be80, 32 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

#define DAT_0089e7c0 (*(undefined4 *)0x0089e7c0)
#define DAT_0089e7c4 (*(undefined4 *)0x0089e7c4)

void __fastcall FUN_0040be80(undefined4 *param_1)

{
  if ((DAT_0089e7c0 == 2) && (DAT_0089e7c4 == 0)) {
    param_1[1] = 7;
  }
  *param_1 = 4;
  return;
}


