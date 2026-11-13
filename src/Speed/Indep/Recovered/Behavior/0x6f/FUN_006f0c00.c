// Original UG2 PC v1.2 0x6f0c00, 63 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



int * __fastcall FUN_006f0c00(void *_this,int _edx_unused,ushort param_1)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar3 = *(int *)_this + -1;
  if (-1 < iVar3) {
    iVar4 = 0;
    do {
      iVar2 = iVar3 + iVar4 >> 1;
      uVar1 = *(ushort *)((int)_this + iVar2 * 8 + 4);
      if (uVar1 < param_1) {
        iVar4 = iVar2 + 1;
      }
      else {
        if (uVar1 <= param_1) {
          return (int *)((int)_this + iVar2 * 8 + 4);
        }
        iVar3 = iVar2 + -1;
      }
    } while (iVar4 <= iVar3);
  }
  return (int *)0x0;
}


