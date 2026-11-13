// Original UG2 PC v1.2 0x6dd1c0, 117 bytes. BEHAVIOUR-VERIFIED ONLY (28 clean trials, 12 both-fault):
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

#define DAT_008a8134 (*(undefined4 *)0x008a8134)
#define PTR_FUN_0080bfdc (*(code *)0x0080bfdc)

void FUN_006dd1c0(void)

{
  int iVar1;
  int *piVar2;
  
  if (DAT_008a8134 != (int *)0x0) {
    piVar2 = DAT_008a8134;
    do {
      if (piVar2[1] != 0) {
        iVar1 = *(int *)(*(int *)(piVar2[1] + 0x24) + 0x1c);
        if (iVar1 == 0) {
          if (piVar2[2] != 0) {
            *(int *)(piVar2[2] + 0xc) = piVar2[3];
          }
          if (piVar2[3] != 0) {
            *(int *)(piVar2[3] + 8) = piVar2[2];
          }
          if (DAT_008a8134 == piVar2) {
            DAT_008a8134 = (int *)piVar2[3];
          }
          *piVar2 = 0;
          piVar2[1] = 0;
          piVar2[2] = 0;
          piVar2[3] = 0;
          (*(code *)PTR_FUN_0080bfdc)(piVar2,0x10);
          piVar2 = (int *)0x0;
        }
        else {
          *(int *)(*piVar2 + 0x48) = iVar1;
        }
      }
      piVar2 = (int *)piVar2[3];
    } while (piVar2 != (int *)0x0);
  }
  return;
}


