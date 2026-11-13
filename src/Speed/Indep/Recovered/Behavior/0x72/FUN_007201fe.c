// Original UG2 PC v1.2 0x7201fe, 62 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



int FUN_007201fe(int *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = param_1[5];
  iVar2 = *param_1;
  if ((iVar2 <= iVar1) && (iVar1 <= param_1[1])) {
    return iVar1;
  }
  if (0 < param_1[4]) {
    param_1[2] = param_1[2] + (int)(char)param_1[3];
    if (param_1[1] < param_1[2]) {
      param_1[2] = iVar2;
    }
    else if (param_1[2] < iVar2) {
      param_1[2] = param_1[1];
    }
  }
  return param_1[2];
}


