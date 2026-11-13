// Original UG2 PC v1.2 0x447970, 82 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



void __fastcall FUN_00447970(void *_this,int _edx_unused,undefined4 *param_1)

{
  *(undefined1 *)((int)_this + 0x2b) = 1;
  if (0x1000 < (int)(((uint)*(ushort *)((int)param_1 + 2) - *(int *)((int)_this + 200)) +
                    *(int *)((int)_this + 0xcc))) {
    *(int *)((int)_this + 0xcc) = *(int *)((int)_this + 200);
  }
  *(undefined4 *)(*(int *)((int)_this + 0xc4) + *(int *)((int)_this + 0xcc)) = *param_1;
  *(int *)((int)_this + 0xcc) = *(int *)((int)_this + 0xcc) + 4;
  return;
}


