// Original UG2 PC v1.2 0x556870, 186 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int data;

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
int __cdecl FUN_0052cf60();
int __cdecl FUN_00555f10();
int __stdcall FUN_0054e6e0(int,int);
#define DAT_008384d8 (*(undefined4 *)0x008384d8)

void __fastcall FUN_00556870(int param_1)

{
  int iVar1;
  void *data;
  
  if (*(char *)(*(int *)(param_1 + 0x58) + 0x4c) != '\0') {
    iVar1 = FUN_0052cf60(*(char **)(param_1 + 0x16c));
    if (((((char)iVar1 == '\0') && (*(char *)(param_1 + 0x172) == '\0')) &&
        (iVar1 = *(int *)(param_1 + 0x58), iVar1 != 0)) &&
       ((((*(char *)(iVar1 + 0x4c) != '\0' && (0x33 < *(int *)(iVar1 + 0x48))) &&
         (*(int *)(iVar1 + 0x48) < 0x3f)) ||
        (((iVar1 != 0 && (*(char *)(iVar1 + 0x4c) != '\0')) &&
         ((0x60 < *(int *)(iVar1 + 0x48) && (*(int *)(iVar1 + 0x48) < 0x99)))))))) {
      if ((iVar1 == 0) || (*(char *)(iVar1 + 0x4c) == '\0')) {
        data = (void *)0xa9;
      }
      else {
        data = *(void **)(iVar1 + 0x48);
      }
      FUN_00555f10(*(char **)(param_1 + 0x16c),*(char **)(param_1 + 4),-1,data);
      FUN_0054e6e0(DAT_008384d8,0x800421,*(char **)(param_1 + 4));
      *(undefined4 *)(param_1 + 0x168) = 0;
    }
  }
  return;
}


