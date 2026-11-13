// Original UG2 PC v1.2 0x673a72, 129 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0075c6c8();


undefined4 * __fastcall FUN_00673a72(void *_this,int _edx_unused,int param_1)

{
  undefined4 *puVar1;
  uint uVar2;
  uint uVar3;
  
  uVar2 = param_1 + 2;
  if (*(uint *)((int)_this + 0x8c) < uVar2) {
    uVar3 = uVar2;
    if (uVar2 < 0x2000) {
      uVar3 = 0x2000;
    }
    puVar1 = FUN_0075c6c8(uVar3 << 2);
    if (puVar1 == (undefined4 *)0x0) {
      return (undefined4 *)0x0;
    }
    *puVar1 = *(undefined4 *)((int)_this + 0x88);
    puVar1[1] = -*(int *)((int)_this + 0x8c);
    *(undefined4 **)((int)_this + 0x88) = puVar1;
    *(uint *)((int)_this + 0x8c) = uVar3;
  }
  else {
    puVar1 = *(undefined4 **)((int)_this + 0x88);
    *puVar1 = puVar1;
    puVar1[1] = *(undefined4 *)((int)_this + 0x8c);
  }
  *(int *)((int)_this + 0x8c) = *(int *)((int)_this + 0x8c) - uVar2;
  *(int *)((int)_this + 0x88) = *(int *)((int)_this + 0x88) + uVar2 * 4;
  return puVar1 + 2;
}


