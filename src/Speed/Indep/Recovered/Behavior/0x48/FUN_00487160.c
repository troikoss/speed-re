// Original UG2 PC v1.2 0x487160, 39 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



void __fastcall FUN_00487160(void *_this, int _edx_unused, undefined4 *param_1, undefined4 param_2, int _pad0, int _pad1, int _pad2)

{
  *(undefined4 *)_this = param_2;
  *param_1 = *(undefined4 *)_this;
  param_1[1] = *(undefined4 *)((int)_this + 4);
  param_1[2] = *(undefined4 *)((int)_this + 8);
  param_1[3] = *(undefined4 *)((int)_this + 0xc);
  return;
}


