// Original UG2 PC v1.2 0x4c51f0, 129 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_004a3100();
int __cdecl FUN_004c4f50();


void __fastcall FUN_004c51f0(void *_this, int _edx_unused, uint param_1, undefined4 param_2, uint param_3, int _pad0)

{
  if (0x8867412d < param_1) {
    if ((param_1 == 0xc98356ba) && (*(char *)((int)_this + 0x4c) == '\0')) {
      FUN_004a3100(*(byte **)((int)_this + 4),*(byte **)((int)_this + 4),0x121,0,0,0,0);
      *(undefined1 *)((int)_this + 0x4c) = 1;
    }
    return;
  }
  if (param_1 != 0x8867412d) {
    if (param_1 == 0x35f8620b) {
      FUN_004a3100(*(byte **)((int)_this + 4),*(byte **)((int)_this + 4),0x121,0,0,0,0);
      return;
    }
    if (param_1 != 0x461a18ee) {
      return;
    }
  }
  FUN_004c4f50(*(char **)((int)_this + 4),param_3);
  return;
}


