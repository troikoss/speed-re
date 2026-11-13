// Original UG2 PC v1.2 0x6dc660, 178 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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

#define PTR_FUN_0080bfdc (*(code *)0x0080bfdc)
#define DAT_008a8078 (*(undefined4 *)0x008a8078)
#define DAT_0080f008 (*(undefined4 *)0x0080f008)
#define DAT_008a80f0 (*(undefined4 *)0x008a80f0)
#define DAT_008a8100 (*(undefined4 *)0x008a8100)

void FUN_006dc660(int *param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  
  iVar2 = *param_1;
  if (iVar2 != 0) {
    if ((1 < *(uint *)(iVar2 + 0x24)) && (*(int *)(iVar2 + 0x18) != 0)) {
      (*(code *)PTR_FUN_0080bfdc)(*(int *)(iVar2 + 0x18),*(undefined4 *)(iVar2 + 8));
      *(undefined4 *)(*param_1 + 0x18) = 0;
    }
    *(int *)(*param_1 + 0x24) = *(int *)(*param_1 + 0x24) + -1;
    if (*(int *)(*param_1 + 0x24) == 0) {
      if (*param_1 == (&DAT_008a8078)[param_2]) {
        (&DAT_008a8078)[param_2] = 0;
      }
      iVar2 = 0;
      if (0 < DAT_0080f008) {
        puVar1 = (undefined4 *)*param_1;
        while (*(undefined4 **)(DAT_008a80f0 + iVar2 * 4) != puVar1) {
          iVar2 = iVar2 + 1;
          if (DAT_0080f008 <= iVar2) {
            DAT_008a8100 = DAT_008a8100 + -1;
            return;
          }
        }
        if (puVar1 != (undefined4 *)0x0) {
          (**(code **)*puVar1)(1);
        }
        *param_1 = 0;
        *(undefined4 *)(DAT_008a80f0 + iVar2 * 4) = 0;
      }
      DAT_008a8100 = DAT_008a8100 + -1;
    }
  }
  return;
}


