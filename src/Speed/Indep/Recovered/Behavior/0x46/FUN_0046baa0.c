// Original UG2 PC v1.2 0x46baa0, 111 bytes. BEHAVIOUR-VERIFIED ONLY (29 clean trials, 11 both-fault):
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
int __cdecl FUN_004691c0();
#define s_FE_MASK____x__IG_Mask____x__NUMF_0078a8f0 ((char *)0x0078a8f0)

void __fastcall FUN_0046baa0(void *_this,int _edx_unused,void *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  *(void **)((int)_this + 0x84) = param_1;
  *(undefined4 *)((int)_this + 0x8c) = param_2;
  *(undefined4 *)((int)_this + 0x90) = param_2;
  *(void **)((int)_this + 0x88) = param_1;
  *(undefined4 *)((int)_this + 0x94) = param_3;
  *(undefined4 *)((int)_this + 0x98) = param_4;
  *(undefined4 *)((int)_this + 0x9c) = param_5;
  *(undefined4 *)((int)_this + 0xa8) = 0xffffffff;
  *(undefined4 *)((int)_this + 0xac) = 0xffffffff;
  FUN_004691c0(0,s_FE_MASK____x__IG_Mask____x__NUMF_0078a8f0,param_1);
  return;
}


