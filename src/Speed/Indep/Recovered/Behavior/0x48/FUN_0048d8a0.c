// Original UG2 PC v1.2 0x48d8a0, 66 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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



uint __fastcall FUN_0048d8a0(void *_this,int _edx_unused,uint param_1)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = *(int *)((int)_this + 0xc);
  if (iVar1 != 0) {
    uVar2 = *(uint *)(iVar1 + 0x80);
    if ((uVar2 != 0) && (*(char *)(iVar1 + 0x1b) != 0)) {
      if (param_1 == 0) {
        return uVar2;
      }
      if ((uVar2 <= param_1) && (param_1 < (*(char *)(iVar1 + 0x1b) * 5 + -5) * 0x10 + uVar2)) {
        return param_1 + 0x50;
      }
    }
  }
  return 0;
}


