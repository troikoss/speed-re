// Original UG2 PC v1.2 0x49c600, 137 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0055dda0();
int __cdecl FUN_005c1000();
int __stdcall FUN_0051fe70(int);
#define DAT_0083aa04 (*(undefined4 *)0x0083aa04)
#define DAT_0083a9d8 (*(undefined4 *)0x0083a9d8)
#define DAT_00836364 (*(undefined4 *)0x00836364)

void __fastcall FUN_0049c600(int param_1)

{
  int *piVar1;
  undefined4 uVar2;
  
  if (DAT_0083aa04 == 3) {
    piVar1 = (int *)FUN_0051fe70((void *)param_1,*(int *)(param_1 + 0x154));
    uVar2 = FUN_005c1000((&DAT_0083a9d8)[DAT_00836364]);
    if ((char)uVar2 == '\0') {
      if ((char)piVar1[10] != '\0') {
        if (*(int **)(param_1 + 0x54) == piVar1) {
          FUN_0055dda0(0x911c0a4b,*(char **)(param_1 + 4));
        }
        (**(code **)(*piVar1 + 0x20))();
                    /* WARNING: Could not recover jumptable at 0x0049c683. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (**(code **)(*piVar1 + 0xc))();
        return;
      }
    }
    else if ((char)piVar1[10] == '\0') {
      (**(code **)(*piVar1 + 0x1c))();
      if (piVar1 == *(int **)(param_1 + 0x54)) {
        (**(code **)(*piVar1 + 0x24))(*(undefined4 *)(param_1 + 4));
      }
                    /* WARNING: Could not recover jumptable at 0x0049c659. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar1 + 0xc))();
      return;
    }
  }
  return;
}


