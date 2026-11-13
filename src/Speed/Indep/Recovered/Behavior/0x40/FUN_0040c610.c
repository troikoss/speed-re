// Original UG2 PC v1.2 0x40c610, 151 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



void __fastcall FUN_0040c610(void *_this,int _edx_unused,undefined4 *param_1,undefined4 param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  undefined2 *puVar5;
  undefined2 *puVar6;
  
  *(undefined4 *)((int)_this + 4) = param_1[0xb];
  *(undefined1 *)((int)_this + 0xbc) = *(undefined1 *)(param_1 + 0xd);
  iVar4 = 0;
  *(undefined1 *)((int)_this + 0xb8) = 0;
  *(undefined4 *)((int)_this + 8) = param_2;
  *(undefined4 *)((int)_this + 0xc) = 0;
  *(undefined4 *)_this = *param_1;
  *(undefined1 *)((int)_this + 0xbb) = *(undefined1 *)(param_1 + 1);
  puVar6 = (undefined2 *)((int)_this + 0x38);
  puVar5 = (undefined2 *)((int)param_1 + 6);
  while( true ) {
    iVar3 = param_1[10];
    if (0x3f < iVar3) {
      iVar3 = 0x40;
    }
    if (iVar3 <= iVar4) break;
    *puVar6 = *puVar5;
    iVar4 = iVar4 + 1;
    puVar5 = puVar5 + 1;
    puVar6 = puVar6 + 1;
  }
  iVar4 = param_1[10];
  if (0x3f < iVar4) {
    iVar4 = 0x40;
  }
  *(char *)((int)_this + 0xb9) = (char)iVar4;
  uVar1 = param_3[1];
  uVar2 = param_3[2];
  *(undefined4 *)((int)_this + 0x10) = *param_3;
  *(undefined4 *)((int)_this + 0x14) = uVar1;
  *(undefined4 *)((int)_this + 0x18) = uVar2;
  *(undefined1 *)((int)_this + 0xba) = 1;
  return;
}


