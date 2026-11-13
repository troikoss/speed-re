// Original UG2 PC v1.2 0x73ee20, 118 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_0073d820();
int __cdecl FUN_0073d8c0();
int __cdecl FUN_0073d8e0();


void FUN_0073ee20(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  if ((param_1 != 0) && (*(int *)(param_1 + 0xc) != 0)) {
    iVar2 = FUN_0073d8c0(*(int *)(param_1 + 0xc));
    while (iVar2 = iVar2 + -1, -1 < iVar2) {
      iVar3 = FUN_0073d8e0(*(int *)(param_1 + 0xc),iVar2);
      if ((iVar3 != 0) && ((param_2 == 0 || ((*(byte *)(iVar3 + 0x10) & 1) != 0)))) {
        iVar3 = *(int *)(param_1 + 0x10);
        if ((-1 < iVar2) && (iVar4 = FUN_0073d820(*(int *)(param_1 + 0xc),iVar2), iVar4 != 0)) {
          (**(code **)(iVar3 + 4))(*(undefined4 *)(iVar4 + 0x14));
          (**(code **)(iVar3 + 4))(iVar4);
        }
        piVar1 = (int *)(*(int *)(param_1 + 0x10) + 0x4c);
        *piVar1 = *piVar1 + -1;
      }
    }
  }
  return;
}


