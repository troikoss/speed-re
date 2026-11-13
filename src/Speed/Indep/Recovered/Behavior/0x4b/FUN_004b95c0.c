// Original UG2 PC v1.2 0x4b95c0, 135 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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
int __cdecl FUN_0049d950();
int __cdecl FUN_004b93a0();
int __cdecl FUN_005379a0();
int __cdecl FUN_0055dde0();
int __cdecl FUN_005b7c40();
#define DAT_00864f4c (*(undefined4 *)0x00864f4c)

void __fastcall FUN_004b95c0(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)((int)_this + 0x58);
  *(undefined1 *)(iVar1 + 0x6c) = 0;
  *(undefined1 *)(iVar1 + 0x6d) = 0;
  iVar1 = *(int *)((int)_this + param_1 * 4 + 0x4c);
  if (iVar1 == 3) {
    FUN_004b93a0();
    FUN_005379a0(*(char **)((int)_this + 4));
    return;
  }
  if (iVar1 == 2) {
    FUN_0049d950(*(uint *)((int)_this + 0x58));
    FUN_0055dde0(0xd15504ec,0);
    FUN_005379a0(*(char **)((int)_this + 4));
    return;
  }
  if ((iVar1 == 1) || (iVar1 == 0)) {
    FUN_005b7c40(_this);
    DAT_00864f4c = 1;
  }
  FUN_005379a0(*(char **)((int)_this + 4));
  return;
}


