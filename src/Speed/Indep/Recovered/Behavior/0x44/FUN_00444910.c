// Original UG2 PC v1.2 0x444910, 107 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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

#define DAT_00865154 (*(float *)0x00865154)

void __fastcall FUN_00444910(void *_this,int _edx_unused,float *param_1)

{
  float fVar1;
  float fVar2;
  
  fVar2 = (float)*(ushort *)(*(int *)((int)_this + 0x1c) + 0x264);
  fVar1 = fVar2 * DAT_00865154 + (float)*(ushort *)(*(int *)((int)_this + 0x1c) + 0xc4);
  fVar2 = fVar2 * param_1[9];
  *param_1 = fVar1;
  if (fVar1 != param_1[2]) {
    *(undefined2 *)(param_1 + 0xb) = 2;
  }
  param_1[1] = fVar2;
  if (fVar2 != param_1[3]) {
    *(undefined2 *)(param_1 + 0xb) = 2;
  }
  return;
}


