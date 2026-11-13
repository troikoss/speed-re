// Original UG2 PC v1.2 0x4e2280, 95 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
#define DAT_00850078 (*(undefined4 *)0x00850078)

undefined4 __fastcall FUN_004e2280(void *_this,int _edx_unused,uint param_1,undefined4 param_2,int param_3,undefined4 param_4,uint param_5)

{
  uint uVar1;
  
  if ((DAT_00850078 == 4) && (*(int *)((int)_this + 0x5c) + 1 != param_3)) {
    return 0;
  }
  if (param_1 < 0x5073ef14) {
    if (param_1 == 0x5073ef13) {
      return 6;
    }
    if (param_1 == 0xc407210) {
      return 0;
    }
    if (param_1 == 0x406415e3) {
      return 0;
    }
  }
  else if (param_1 == 0xd9feec59) {
    return 7;
  }
  uVar1 = FUN_0050d1a0(param_1,param_5,'\x01');
  return uVar1;
}


