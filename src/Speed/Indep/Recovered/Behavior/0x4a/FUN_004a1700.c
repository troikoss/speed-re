// Original UG2 PC v1.2 0x4a1700, 76 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int dest;
typedef int format;

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
int __cdecl FUN_004400d0();
int __cdecl FUN_004ffa80();
int __cdecl FUN_00558310();
#define s_animating_007902c8 ((char *)0x007902c8)

void __fastcall FUN_004a1700(void *_this,int _edx_unused,void *param_1,int param_2)

{
  char *format;
  int iVar1;
  char *dest;
  char local_100 [256];
  
  format = FUN_004ffa80(param_1,(uint)param_1);
  dest = local_100;
  FUN_004400d0(dest,format,param_2);
  iVar1 = FUN_00558310(*(char **)((int)_this + 4),s_animating_007902c8,local_100,(int)dest);
  *(int *)((int)_this + 0x4c) = iVar1;
  return;
}


