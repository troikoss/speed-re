// Original UG2 PC v1.2 0x6dd140, 121 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

#define DAT_008a8130 (*(undefined4 *)0x008a8130)
#define PTR_FUN_0080bfdc (*(code *)0x0080bfdc)

void FUN_006dd140(void)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  
  piVar1 = DAT_008a8130;
  while (piVar5 = piVar1, piVar5 != (int *)0x0) {
    piVar1 = (int *)piVar5[3];
    iVar2 = piVar5[2];
    if (piVar5[1] != 0) {
      iVar3 = *(int *)(piVar5[1] + 0x24);
      if ((iVar3 == 0) || (piVar4 = *(int **)(iVar3 + 0x18), piVar4 == (int *)0x0)) {
        if (iVar2 != 0) {
          *(int **)(iVar2 + 0xc) = piVar1;
        }
        if (piVar5[3] != 0) {
          *(int *)(piVar5[3] + 8) = iVar2;
        }
        if (DAT_008a8130 == piVar5) {
          DAT_008a8130 = piVar1;
        }
        *piVar5 = 0;
        piVar5[1] = 0;
        piVar5[2] = 0;
        piVar5[3] = 0;
        (*(code *)PTR_FUN_0080bfdc)(piVar5,0x10);
      }
      else {
        (**(code **)(*piVar4 + 0x3c))(piVar4,0,*piVar5 + 0x44);
      }
    }
  }
  return;
}


