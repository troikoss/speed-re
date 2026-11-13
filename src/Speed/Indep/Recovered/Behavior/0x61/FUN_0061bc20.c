// Original UG2 PC v1.2 0x61bc20, 85 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_0043c140();
int __cdecl FUN_004ff970();
#define DAT_007ea624 (*(undefined4 *)0x007ea624)

int __fastcall FUN_0061bc20(void *_this,int _edx_unused,uint *param_1)

{
  int iVar1;
  uint uVar2;
  
  if (param_1 == (uint *)0x0) {
    param_1 = &DAT_007ea624;
  }
  if (*(int *)((int)_this + 0x844) == 1) {
    iVar1 = FUN_004ff970();
    return (iVar1 != 0) + 1;
  }
  if (*(char *)((int)_this + 0xd9) == '\0') {
    return 0;
  }
  uVar2 = FUN_0043c140((int)*(char *)((int)_this + 0xd9),param_1);
  return (int)*(char *)(uVar2 + 0x880 + (int)_this);
}


