// Original UG2 PC v1.2 0x52a4c0, 266 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int ptr;

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
int __cdecl FUN_00440540();
int __cdecl FUN_00440820();
int __cdecl FUN_005b9680();
int __stdcall FUN_0052a280(int,int);
#define DAT_00838467 (*(undefined4 *)0x00838467)

undefined1 __fastcall FUN_0052a4c0(int param_1)

{
  int iVar1;
  char *ptr;
  int iVar2;
  char *pcVar3;
  char *pcVar4;
  bool bVar5;
  undefined1 local_9;
  
  if (*(int *)(param_1 + 0x16778) == 0) {
    return 0;
  }
  iVar1 = FUN_005b9680();
  ptr = (char *)FUN_00440820(iVar1 + 0xd254,8);
  local_9 = 1;
  iVar1 = FUN_005b9680();
  FUN_0052a280((void *)(param_1 + 0x10),(undefined4 *)ptr,iVar1 + 0xd254);
  if (DAT_00838467 == '\0') {
    iVar1 = FUN_005b9680();
    iVar1 = iVar1 + 0xd254;
    bVar5 = true;
    pcVar3 = ptr;
    pcVar4 = *(char **)(param_1 + 0x16778);
    do {
      if (iVar1 == 0) break;
      iVar1 = iVar1 + -1;
      bVar5 = *pcVar3 == *pcVar4;
      pcVar3 = pcVar3 + 1;
      pcVar4 = pcVar4 + 1;
    } while (bVar5);
    if (bVar5) {
      local_9 = 0;
    }
  }
  else {
    iVar1 = 0;
    while( true ) {
      iVar2 = FUN_005b9680();
      if (iVar2 + 0xd254 <= iVar1) break;
      iVar2 = FUN_005b9680();
      iVar2 = iVar2 + (0xd254 - iVar1);
      if (8 < iVar2) {
        iVar2 = 8;
      }
      bVar5 = true;
      pcVar3 = ptr + iVar1;
      pcVar4 = (char *)(*(int *)(param_1 + 0x16778) + iVar1);
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar5 = *pcVar3 == *pcVar4;
        pcVar3 = pcVar3 + 1;
        pcVar4 = pcVar4 + 1;
      } while (bVar5);
      if (!bVar5) {
        local_9 = 1;
        break;
      }
      local_9 = 0;
      iVar1 = iVar1 + 8;
    }
  }
  FUN_00440540(ptr);
  if (*(void **)(param_1 + 0x16778) != (void *)0x0) {
    FUN_00440540(*(void **)(param_1 + 0x16778));
    *(undefined4 *)(param_1 + 0x16778) = 0;
  }
  return local_9;
}


