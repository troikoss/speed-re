// Original UG2 PC v1.2 0x6103f0, 243 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
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



bool __fastcall FUN_006103f0(void *_this,int _edx_unused,int param_1)

{
  byte bVar1;
  int iVar2;
  
  if ((((param_1 < 0x23) || (0x2e < param_1)) || (*(int *)((int)_this + 0x618) == 0)) ||
     (bVar1 = *(byte *)(*(int *)((int)_this + 0x618) + 5) >> 5, bVar1 == 0)) {
    return false;
  }
  if (bVar1 == 1) {
    if ((*(int *)_this < 8) || (iVar2 = 3, 10 < *(int *)_this)) {
      iVar2 = 1;
    }
    return param_1 <= iVar2 + 0x23;
  }
  if (bVar1 == 2) {
    if ((*(int *)_this < 8) || (10 < *(int *)_this)) {
      return param_1 < 0x27;
    }
switchD_00610456_caseD_5:
    return param_1 < 0x29;
  }
  if (bVar1 != 3) {
    return param_1 < 0x24;
  }
  switch(*(undefined4 *)_this) {
  case 5:
  case 0xd:
  case 0xf:
  case 0x12:
  case 0x16:
  case 0x19:
  case 0x1a:
  case 0x1c:
  case 0x1e:
    goto switchD_00610456_caseD_5;
  default:
    return param_1 < 0x28;
  case 8:
  case 9:
  case 10:
    return param_1 < 0x2f;
  }
}


