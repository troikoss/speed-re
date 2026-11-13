// Original UG2 PC v1.2 0x5d43c0, 73 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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

#define DAT_00883db4 (*(undefined4 *)0x00883db4)
#define DAT_00883db0 (*(undefined4 *)0x00883db0)

int __fastcall FUN_005d43c0(void *_this,int _edx_unused,int param_1)

{
  uint uVar1;
  ushort *puVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = 0;
  iVar3 = 0;
  puVar2 = (ushort *)((int)_this + 0x1c);
  do {
    uVar1 = (uint)*puVar2;
    if ((uVar1 != 0xffff) && ((int)uVar1 < DAT_00883db4)) {
      if (param_1 == iVar4) {
        return uVar1 * 0x20 + DAT_00883db0;
      }
      iVar4 = iVar4 + 1;
    }
    iVar3 = iVar3 + 1;
    puVar2 = puVar2 + 1;
  } while (iVar3 < 2);
  return 0;
}


