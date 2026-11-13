// Original UG2 PC v1.2 0x6f937d, 94 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_006f9271();


undefined4 FUN_006f937d(undefined4 param_1,int *param_2)

{
  int *piVar1;
  undefined4 extraout_EAX;
  int iVar2;
  int iVar3;
  undefined4 local_8;
  
  piVar1 = param_2;
  iVar2 = 0;
  local_8 = 1;
  if (0 < *param_2) {
    do {
      iVar3 = iVar2 * 4;
      FUN_006f9271(*(char **)(iVar3 + piVar1[1]),&param_2,piVar1[4]);
      *(undefined4 *)(iVar3 + piVar1[2]) = extraout_EAX;
      if (*(int *)(iVar3 + piVar1[2]) == 0) {
        local_8 = 0;
      }
      if (piVar1[3] != 0) {
        *(int **)(piVar1[3] + iVar3) = param_2;
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < *piVar1);
  }
  return local_8;
}


