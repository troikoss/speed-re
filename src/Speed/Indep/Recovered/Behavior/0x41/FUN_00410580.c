// Original UG2 PC v1.2 0x410580, 45 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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

#define DAT_007e95c4 (*(undefined4 *)0x007e95c4)
#define DAT_00883db0 (*(undefined4 *)0x00883db0)

void __fastcall FUN_00410580(void *_this,int _edx_unused,int param_1)

{
  if ((DAT_007e95c4 != 0) && ((*(byte *)(param_1 + 0xe) & 3) == 2)) {
    *(uint *)((int)_this + 0x750) = param_1 - DAT_00883db0 >> 5 & 0xffff;
  }
  return;
}


