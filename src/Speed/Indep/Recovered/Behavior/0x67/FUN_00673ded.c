// Original UG2 PC v1.2 0x673ded, 163 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0075dc60();


void __fastcall FUN_00673ded(void *_this,int _edx_unused,double *param_1,double *param_2,double *param_3,byte param_4)

{
  double *pdVar1;
  int iVar2;
  uint uVar3;
  double local_24 [4];
  
  local_24[0] = *param_2 * *param_3;
  uVar3 = 0;
  local_24[1] = *param_2 * param_3[1];
  local_24[2] = *param_3 * param_2[1];
  local_24[3] = param_2[1] * param_3[1];
  *param_1 = *(double *)((int)_this + 0xa0);
  param_1[1] = *(double *)((int)_this + 0xa8);
  do {
    pdVar1 = local_24 + uVar3;
    iVar2 = FUN_0075dc60(*pdVar1);
    if (iVar2 != 0) {
      if ((param_4 & 0x40) != 0) {
        *param_1 = local_24[uVar3];
        param_1[1] = local_24[uVar3];
        return;
      }
      *pdVar1 = 0.0;
    }
    if (*pdVar1 < *param_1) {
      *param_1 = *pdVar1;
    }
    if (param_1[1] < *pdVar1) {
      param_1[1] = *pdVar1;
    }
    uVar3 = uVar3 + 1;
  } while (uVar3 < 4);
  return;
}


