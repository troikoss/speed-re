// Original UG2 PC v1.2 0x488f10, 133 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int __cdecl FUN_00489200();


undefined4 FUN_00488f10(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int local_400 [256];
  
  if (((param_1 == 0) || (0xff < param_2)) || (param_2 < 0)) {
    return 0;
  }
  if (param_2 == 0) {
    return 1;
  }
  iVar4 = 0;
  iVar3 = 0;
  if (0 < param_2) {
    do {
      iVar1 = *(int *)(param_1 + iVar3 * 4);
      if (((iVar1 != 0) && (iVar1 = *(int *)(iVar1 + 0xc), iVar1 != 0)) &&
         (*(int *)(iVar1 + 0x84) != 0)) {
        local_400[iVar4] = iVar1;
        iVar4 = iVar4 + 1;
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < param_2);
  }
  uVar2 = FUN_00489200(local_400,iVar4);
  return uVar2;
}


