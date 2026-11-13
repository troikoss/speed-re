// Original UG2 PC v1.2 0x488e80, 45 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



void __fastcall FUN_00488e80(void *_this,int _edx_unused,undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  iVar2 = *(int *)((int)_this + 0xc);
  if (iVar2 != 0) {
    uVar3 = *(undefined4 *)(iVar2 + 0x7c);
    uVar1 = *(undefined4 *)(iVar2 + 0x78);
    uVar4 = *(undefined4 *)(iVar2 + 0x70);
    param_1[1] = *(undefined4 *)(iVar2 + 0x74);
    *param_1 = uVar4;
    param_1[2] = uVar1;
    param_1[3] = uVar3;
  }
  return;
}


