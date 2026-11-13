// Original UG2 PC v1.2 0x4a33d0, 121 bytes. BEHAVIOUR-VERIFIED ONLY (21 clean trials, 19 both-fault):
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
int __cdecl FUN_0050d1a0();
int __cdecl FUN_0050d1d0();
int __cdecl FUN_0050d1f0();
int __cdecl FUN_0050d210();


undefined4 __fastcall FUN_004a33d0(void *_this, int _edx_unused, int param_1, int _pad0, int _pad1, int _pad2, int _pad3)

{
  uint uVar1;
  int iVar2;
  uint in_stack_00000014;
  
  if (*(char *)((int)_this + 100) != '\0') {
    if (param_1 == 0xc407210) {
      return 8;
    }
    switch(*(undefined4 *)((int)_this + 0x8c)) {
    case 1:
      uVar1 = FUN_0050d1a0(param_1,in_stack_00000014,'\x01');
      return uVar1;
    case 2:
      iVar2 = FUN_0050d1d0(param_1,in_stack_00000014);
      return iVar2;
    case 3:
      uVar1 = FUN_0050d210(param_1,in_stack_00000014);
      return uVar1;
    case 4:
      uVar1 = FUN_0050d1f0(param_1,in_stack_00000014);
      return uVar1;
    }
  }
  return 0;
}


