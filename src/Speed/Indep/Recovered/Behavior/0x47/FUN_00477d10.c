// Original UG2 PC v1.2 0x477d10, 67 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int param;

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
int __cdecl FUN_004691c0();
#define s_Sub_Channel_Array_Ptr__x__Curren_0078b73c ((char *)0x0078b73c)

int __fastcall FUN_00477d10(void *_this,int _edx_unused,int param_1)

{
  void *param;
  
  param = (void *)(*(int *)((int)_this + 0x88) + *(int *)((int)_this + 0x9c));
  FUN_004691c0(0,s_Sub_Channel_Array_Ptr__x__Curren_0078b73c,param);
  *(int *)((int)_this + 0x9c) = *(int *)((int)_this + 0x9c) + param_1 * 4;
  return (int)param;
}


