// Original UG2 PC v1.2 0x52a460, 91 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_00440820();
int __cdecl FUN_005b9680();
int __stdcall FUN_0052a280(int,int);


void __fastcall FUN_0052a460(void *_this,int _edx_unused,char param_1)

{
  int iVar1;
  
  if ((*(int *)((int)_this + 0x16778) == 0) &&
     ((param_1 != '\0' || (*(char *)((int)_this + 0xa108) != '\0')))) {
    iVar1 = FUN_005b9680();
    iVar1 = FUN_00440820(iVar1 + 0xd254,8);
    *(int *)((int)_this + 0x16778) = iVar1;
    iVar1 = FUN_005b9680();
    FUN_0052a280((void *)((int)_this + 0x10),*(undefined4 **)((int)_this + 0x16778),iVar1 + 0xd254);
  }
  return;
}


