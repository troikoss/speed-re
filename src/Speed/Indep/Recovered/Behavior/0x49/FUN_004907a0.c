// Original UG2 PC v1.2 0x4907a0, 61 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
int __cdecl FUN_004901d0();
#define DAT_00864f60 (*(undefined4 *)0x00864f60)

void __fastcall FUN_004907a0(int param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  
  if ((DAT_00864f60 == 0) && (iVar3 = 0, '\0' < *(char *)(param_1 + 0x19))) {
    do {
      iVar1 = *(int *)(param_1 + 0x2c);
      uVar2 = FUN_004901d0(*(uint *)(iVar1 + iVar3 * 8),1,0);
      *(uint *)(iVar1 + iVar3 * 8 + 4) = uVar2;
      iVar3 = iVar3 + 1;
    } while (iVar3 < *(char *)(param_1 + 0x19));
  }
  return;
}


