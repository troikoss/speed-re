// Original UG2 PC v1.2 0x5daa40, 124 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_0048b210();
int __cdecl FUN_00577c70();
#define DAT_0082daa8 (*(undefined4 *)0x0082daa8)

void __fastcall FUN_005daa40(void *_this,int _edx_unused,int param_1)

{
  char *local_8;
  int local_4;
  
  *(int *)((int)_this + 0x24) = *(int *)((int)_this + 0x24) + 1;
  DAT_0082daa8 = DAT_0082daa8 + 1;
  FUN_0048b210(0);
  local_8 = *(char **)(param_1 + 0x44);
  local_4 = *(int *)(param_1 + 0x4c);
  FUN_00577c70(&local_8,&local_4,0x2011,(char *)param_1);
  *(int *)(param_1 + 0x4c) = local_4;
  *(char **)(param_1 + 0x44) = local_8;
  *(undefined4 *)(param_1 + 0xc) = 4;
  *(undefined4 *)(param_1 + 0x3c) = 0;
  DAT_0082daa8 = DAT_0082daa8 + -1;
  FUN_0048b210(1);
  return;
}


