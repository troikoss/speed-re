// Original UG2 PC v1.2 0x6b7ef4, 69 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



undefined4 __fastcall FUN_006b7ef4(void *_this,int _edx_unused,int param_1,uint *param_2,undefined4 *param_3)

{
  byte bVar1;
  uint uVar2;
  undefined4 uVar3;
  
  uVar2 = *(uint *)(param_1 + 0x6c);
  bVar1 = *(byte *)(param_1 + 0x6d);
  *param_2 = (uint)bVar1;
  uVar2 = uVar2 & 0xff;
  uVar3 = 1;
  if (uVar2 == 1) {
    uVar2 = *(uint *)((int)_this + 0x28);
  }
  else {
    uVar3 = 3;
    if (uVar2 != 3) {
      return 0x80004005;
    }
    uVar2 = *(uint *)((int)_this + 0x3c);
  }
  if (bVar1 < uVar2) {
    *param_3 = uVar3;
    return 0;
  }
  return 0x80004005;
}


