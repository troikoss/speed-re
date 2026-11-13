// Original UG2 PC v1.2 0x5753c0, 107 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_0043dc00();


char * __fastcall FUN_005753c0(void *_this,int _edx_unused,undefined4 *param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = _this;
  for (iVar1 = 0x20; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  FUN_0043dc00(_this,(char *)param_2,0x7f);
  *(undefined4 *)((int)_this + 0x80) = *param_1;
  *(undefined4 *)((int)_this + 0x84) = param_1[1];
  *(undefined4 *)((int)_this + 0x88) = param_1[2];
  *(undefined4 *)((int)_this + 0x8c) = param_1[3];
  iVar1 = param_1[4];
  *(int *)((int)_this + 0x90) = iVar1;
  *(undefined4 *)((int)_this + 0x98) = 0;
  *(undefined4 *)((int)_this + 0x9c) = 0;
  *(int *)((int)_this + 0x94) = iVar1 >> 0x1f;
  return _this;
}


