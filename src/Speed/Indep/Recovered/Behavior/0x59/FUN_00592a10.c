// Original UG2 PC v1.2 0x592a10, 67 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



void __fastcall FUN_00592a10(void *_this,int _edx_unused,undefined4 *param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar1 = *(undefined4 *)((int)_this + 0xe4);
  uVar2 = *(undefined4 *)((int)_this + 0xe8);
  *param_1 = *(undefined4 *)((int)_this + 0xe0);
  param_1[1] = uVar1;
  param_1[2] = uVar2;
  uVar1 = *(undefined4 *)((int)_this + 0xf4);
  uVar2 = *(undefined4 *)((int)_this + 0xf8);
  *param_2 = *(undefined4 *)((int)_this + 0xf0);
  param_2[1] = uVar1;
  param_2[2] = uVar2;
  return;
}


