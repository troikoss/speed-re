// Original UG2 PC v1.2 0x7201c2, 60 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_007356d0();


undefined4 FUN_007201c2(int param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = param_1;
  iVar2 = *(int *)(param_1 + 0x14);
  if (*(int *)(param_1 + 0x10) != iVar2) {
    piVar1 = (int *)(param_1 + 0xc);
    *(int *)(param_1 + 0x10) = iVar2;
    if (iVar2 < *(int *)(param_1 + 8)) {
      param_1 = *(int *)(param_1 + 8);
    }
    else {
      param_1 = *piVar1;
      if (iVar2 <= *piVar1) {
        param_1 = iVar2;
      }
    }
    FUN_007356d0((int *)iVar3,&param_1);
  }
  return 0;
}


