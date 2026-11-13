// Original UG2 PC v1.2 0x5769d0, 62 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



void __fastcall FUN_005769d0(void *_this,int _edx_unused,float param_1)

{
  int iVar1;
  
  iVar1 = 0;
  if (*(char *)((int)_this + 5) != '\0') {
    do {
      *(float *)(*(int *)((int)_this + 0x10) + iVar1 * 4) = param_1;
      iVar1 = iVar1 + 1;
    } while (iVar1 < (int)(uint)*(byte *)((int)_this + 5));
  }
  *(undefined1 *)((int)_this + 6) = 0;
  *(float *)((int)_this + 8) = (float)*(byte *)((int)_this + 5) * param_1;
  return;
}


