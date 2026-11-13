// Original UG2 PC v1.2 0x4fe970, 116 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.

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
int __cdecl FUN_0043c1c0();
int __cdecl FUN_0075c6c8();
int __cdecl FUN_0075c6da();
#define PTR_DAT_007f6cd0 (*(undefined4 *)0x007f6cd0)

void FUN_004fe970(int param_1,int param_2)

{
  undefined4 *_Memory;
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  
  _Memory = FUN_0075c6c8(0xd4);
  puVar3 = _Memory;
  for (iVar2 = 0x35; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  iVar2 = 0;
  if (0 < param_2) {
    do {
      iVar4 = 0;
      do {
        uVar1 = FUN_0043c1c0(0x35);
        if (_Memory[uVar1] == 0) {
          *(undefined **)(param_1 + iVar2 * 4) = (&PTR_DAT_007f6cd0)[uVar1];
          _Memory[uVar1] = 1;
          break;
        }
        iVar4 = iVar4 + 1;
      } while (iVar4 < 100);
      iVar2 = iVar2 + 1;
    } while (iVar2 < param_2);
  }
  FUN_0075c6da(_Memory);
  return;
}


