// Original UG2 PC v1.2 0x40af30, 141 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 __fastcall FUN_0040af30(void *_this,int _edx_unused,int param_1,int param_2,float param_3)

{
  int iVar1;
  int iVar2;
  
  if (param_1 < 0) {
    if (1 < param_1) goto LAB_0040af4b;
LAB_0040af57:
    iVar2 = 0;
  }
  else if (param_1 < 2) {
    iVar2 = 1;
  }
  else {
LAB_0040af4b:
    if (9 < param_1) goto LAB_0040af57;
    iVar2 = 2;
  }
  if (param_2 < 0) {
    if (1 < param_2) goto LAB_0040af72;
  }
  else {
    if (param_2 < 2) {
      iVar1 = 1;
      goto LAB_0040af80;
    }
LAB_0040af72:
    if (param_2 < 10) {
      iVar1 = 2;
      goto LAB_0040af80;
    }
  }
  iVar1 = 0;
LAB_0040af80:
  if ((iVar2 != 2) || (iVar1 != 2)) {
    return 1;
  }
  if (param_3 <=
      *(float *)(*(int *)((int)_this + 100) + 0xac + param_1 * 4) -
      *(float *)(*(int *)((int)_this + 100) + 0xac + param_2 * 4)) {
    return 1;
  }
  return 0;
}


