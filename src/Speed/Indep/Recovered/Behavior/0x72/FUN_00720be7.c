// Original UG2 PC v1.2 0x720be7, 115 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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
int __cdecl FUN_007207e9();
int __cdecl FUN_0072085b();
int __cdecl FUN_00725480();


void FUN_00720be7(int param_1)

{
  int iVar1;
  uint uVar2;
  char *pcVar3;
  int iVar4;
  int iVar5;
  
  iVar1 = param_1;
  iVar4 = 0x1000;
  iVar5 = 0x7fff;
  uVar2 = FUN_00725480(*(uint *)(param_1 + 8));
  if (-1 < (int)uVar2) {
    param_1 = 0;
    pcVar3 = (char *)(iVar1 + 0x1c);
    if (*(char *)(iVar1 + 0xe) != '\0') {
      do {
        if (*pcVar3 == '\0') {
          iVar4 = *(int *)(pcVar3 + 8);
          *(int *)(pcVar3 + 4) = iVar4;
        }
        else if (*pcVar3 == '\x02') {
          iVar5 = *(int *)(pcVar3 + 8);
          *(int *)(pcVar3 + 4) = iVar5;
        }
        pcVar3 = pcVar3 + 0xc;
        param_1 = param_1 + 1;
      } while (param_1 < (int)(uint)*(byte *)(iVar1 + 0xe));
    }
    FUN_007207e9(uVar2,iVar4);
    FUN_0072085b(uVar2,iVar5);
  }
  return;
}


