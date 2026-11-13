// Original UG2 PC v1.2 0x522050, 33 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_0083899c (*(undefined4 *)0x0083899c)
#define DAT_008389a0 (*(undefined4 *)0x008389a0)

void __fastcall FUN_00522050(void *_this, int _edx_unused, undefined4 param_1, int param_2, int _pad0, int _pad1, int _pad2)

{
  if (param_2 == 0xc407210) {
    DAT_0083899c = *(undefined4 *)((int)_this + 0x48);
    DAT_008389a0 = *(undefined4 *)((int)_this + 0x4c);
  }
  return;
}


