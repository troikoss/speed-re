// Original UG2 PC v1.2 0x40c440, 138 bytes. BEHAVIOUR-VERIFIED ONLY (24 clean trials, 16 both-fault):
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

#define DAT_0086518c (*(undefined4 *)0x0086518c)

uint __fastcall FUN_0040c440(void *_this,int _edx_unused,uint *param_1,uint *param_2,uint *param_3,uint param_4,uint param_5,
            uint param_6)

{
  uint uVar1;
  uint uVar2;
  
  if (*(uint *)_this == 0) {
    uVar1 = param_1[1];
    uVar2 = param_1[2];
    *(uint *)((int)_this + 0x10) = *param_1;
    *(uint *)((int)_this + 0x14) = uVar1;
    *(uint *)((int)_this + 0x18) = uVar2;
    uVar1 = param_2[1];
    *(uint *)((int)_this + 0x20) = *param_2;
    *(uint *)((int)_this + 0x24) = uVar1;
    uVar1 = param_3[1];
    uVar2 = param_3[2];
    *(uint *)((int)_this + 0x30) = *param_3;
    *(uint *)((int)_this + 0x34) = uVar1;
    *(uint *)((int)_this + 0x38) = uVar2;
    *(uint *)((int)_this + 0x16d4) = param_4;
    *(uint *)((int)_this + 0x16d0) = param_5;
    *(uint *)((int)_this + 0x17a4) = param_6;
    *(undefined4 *)((int)_this + 0x40) = 0;
    *(undefined1 *)((int)_this + 0x17a0) = 0;
    *(undefined4 *)((int)_this + 0x17a8) = DAT_0086518c;
    *(undefined4 *)_this = 1;
    *(undefined4 *)((int)_this + 4) = 1;
    return 1;
  }
  return *(uint *)_this & 0xffffff00;
}


