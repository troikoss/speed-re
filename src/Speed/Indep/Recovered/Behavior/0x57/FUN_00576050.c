// Original UG2 PC v1.2 0x576050, 56 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int cheatdata_field28;

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



void __fastcall FUN_00576050(void *_this,int _edx_unused,int cheatdata_field28)

{
  int iVar1;
  uint uVar2;
  
  if ((cheatdata_field28 != 0) && (uVar2 = 0, *(int *)((int)_this + 0x10) != 0)) {
    iVar1 = 0;
    do {
      if (*(int *)(*(int *)((int)_this + 0xc) + 0x28 + iVar1) == cheatdata_field28) {
        *(undefined1 *)(*(int *)((int)_this + 0xc) + iVar1 + 0x30) = 0;
      }
      uVar2 = uVar2 + 1;
      iVar1 = iVar1 + 0x34;
    } while (uVar2 < *(uint *)((int)_this + 0x10));
  }
  return;
}


