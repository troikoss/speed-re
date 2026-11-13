// Original UG2 PC v1.2 0x72a229, 68 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void FUN_0072a229(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *param_1;
  if (param_2 == iVar2) {
    *param_1 = *(int *)(iVar2 + 8);
    return;
  }
  while ((*(int *)(iVar2 + 8) != 0 && (*(int *)(iVar2 + 8) != param_2))) {
    iVar2 = *(int *)(iVar2 + 8);
  }
  iVar1 = *(int *)(iVar2 + 8);
  if ((iVar1 != 0) && (iVar1 == param_2)) {
    if (*(int *)(iVar1 + 8) != 0) {
      *(int *)(*(int *)(iVar1 + 8) + 0x10) = iVar2;
    }
    *(undefined4 *)(iVar2 + 8) = *(undefined4 *)(*(int *)(iVar2 + 8) + 8);
  }
  return;
}


