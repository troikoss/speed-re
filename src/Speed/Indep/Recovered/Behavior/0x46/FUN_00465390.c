// Original UG2 PC v1.2 0x465390, 106 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



void FUN_00465390(int *param_1,int param_2,int param_3,int param_4,int param_5)

{
  int *piVar1;
  
  if ((((((param_1 != (int *)0x0) && (piVar1 = (int *)*param_1, *piVar1 == 0x4d525453)) &&
        (0 < param_2)) && (param_2 <= piVar1[0xb])) &&
      ((((param_2 != piVar1[0xb] || (param_3 == 0 && param_4 == 0)) &&
        ((0 < param_5 || ((param_5 == -1 || (param_5 == -2)))))) && (param_5 <= piVar1[0xd])))) &&
     (piVar1[0x11] == 0)) {
    piVar1 = (int *)(piVar1[10] + -0xc + param_2 * 0xc);
    *piVar1 = param_3;
    piVar1[1] = param_4;
    piVar1[2] = param_5;
  }
  return;
}


