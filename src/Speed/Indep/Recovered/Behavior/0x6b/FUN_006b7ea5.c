// Original UG2 PC v1.2 0x6b7ea5, 79 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined4 __fastcall FUN_006b7ea5(void *_this,int _edx_unused,int param_1,uint *param_2)

{
  uint uVar1;
  uint uVar2;
  
  uVar2 = (uint)*(byte *)(param_1 + 0x6d);
  uVar1 = *(uint *)(param_1 + 0x6c) & 0xff;
  if (uVar1 == 1) {
    if (*(uint *)((int)_this + 0x28) <= uVar2) {
      return 0x80004005;
    }
  }
  else {
    if (uVar1 != 3) {
      if (uVar1 == 5) {
        *param_2 = 0xc;
        return 0;
      }
      return 0x80004005;
    }
    if (*(uint *)((int)_this + 0x3c) <= uVar2) {
      return 0x80004005;
    }
    uVar2 = uVar2 + 2;
  }
  *param_2 = uVar2;
  return 0;
}


