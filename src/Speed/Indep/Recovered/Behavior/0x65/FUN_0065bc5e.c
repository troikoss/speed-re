// Original UG2 PC v1.2 0x65bc5e, 92 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



uint __fastcall FUN_0065bc5e(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = -(uint)((uint)(*(int *)((int)_this + 0x1c) + *(int *)((int)_this + 0x20)) <
                 (uint)(*(int *)((int)_this + 0x20) + 4 + param_1)) & 0x80004005;
  if (-1 < (int)uVar2) {
    iVar1 = *(int *)(*(int *)((int)_this + 0x20) + param_1);
    if (iVar1 != 0) {
      uVar2 = -(uint)((uint)(*(int *)((int)_this + 0x20) + *(int *)((int)_this + 0x1c)) <
                     (uint)(*(int *)((int)_this + 0x20) + iVar1 + param_1 + 4)) & 0x80004005;
      if ((-1 < (int)uVar2) &&
         (*(char *)(*(int *)((int)_this + 0x20) + iVar1 + -1 + param_1 + 4) != '\0')) {
        uVar2 = 0x80004005;
      }
    }
  }
  return uVar2;
}


