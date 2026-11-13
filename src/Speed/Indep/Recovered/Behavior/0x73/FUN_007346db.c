// Original UG2 PC v1.2 0x7346db, 97 bytes. BEHAVIOUR-VERIFIED ONLY (24 clean trials, 16 both-fault):
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
int __stdcall FUN_0073432c(int,int,int);


undefined4 __fastcall FUN_007346db(void *_this,int _edx_unused,byte *param_1,undefined4 param_2,int param_3)

{
  byte bVar1;
  undefined4 uVar2;
  
  if ((param_1 == (byte *)0x0) || (*(int *)((int)_this + 0xd54) != 0)) {
    uVar2 = 0xffffffff;
  }
  else {
    *(undefined4 *)((int)_this + 0xd44) = 0;
    *(int *)((int)_this + 0xd54) = param_3;
    *(undefined4 *)((int)_this + 0xd58) = param_2;
    *(byte **)((int)_this + 0xd48) = param_1;
    if (*(int *)((int)_this + 0xd64) == 1) {
      bVar1 = *param_1;
      param_1 = param_1 + 1;
      *(uint *)((int)_this + 0xd60) = (uint)(bVar1 == 0xee);
    }
    FUN_0073432c(param_1,_this,*(int *)((int)_this + 0xd5c));
    uVar2 = 0;
  }
  return uVar2;
}


