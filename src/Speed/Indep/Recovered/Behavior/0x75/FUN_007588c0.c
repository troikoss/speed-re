// Original UG2 PC v1.2 0x7588c0, 68 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

#define DAT_007d35b4 (*(undefined4 *)0x007d35b4)

undefined4 FUN_007588c0(char *param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  char *pcVar4;
  int *piVar5;
  bool bVar6;
  
  iVar3 = 0;
  piVar2 = &DAT_007d35b4;
  do {
    iVar1 = *piVar2;
    if (iVar1 <= param_2) {
      bVar6 = true;
      pcVar4 = param_1;
      piVar5 = piVar2 + 1;
      do {
        if (iVar1 == 0) break;
        iVar1 = iVar1 + -1;
        bVar6 = *pcVar4 == (char)*piVar5;
        pcVar4 = pcVar4 + 1;
        piVar5 = (int *)((int)piVar5 + 1);
      } while (bVar6);
      if (bVar6) {
        return *(undefined4 *)(iVar3 * 0x18 + 0x7d35b0);
      }
    }
    piVar5 = piVar2 + 5;
    piVar2 = piVar2 + 6;
    iVar3 = iVar3 + 1;
    if (*piVar5 == 0) {
      return 0;
    }
  } while( true );
}


