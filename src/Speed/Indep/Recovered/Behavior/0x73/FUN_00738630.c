// Original UG2 PC v1.2 0x738630, 110 bytes. BEHAVIOUR-VERIFIED ONLY (21 clean trials, 19 both-fault):
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
int __cdecl FUN_00736060();
int __cdecl FUN_007360a0();
int __cdecl FUN_007363a0();
int __cdecl FUN_00736920();


void FUN_00738630(undefined4 *param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  undefined4 *extraout_EAX;
  uint uVar3;
  undefined4 *puVar4;
  
  iVar2 = FUN_007363a0((short *)&param_2);
  if (iVar2 != 0) {
    cVar1 = *(char *)(iVar2 + 0xb);
    FUN_00736060(cVar1 * 4 + 4);
    if (extraout_EAX != (undefined4 *)0x0) {
      *extraout_EAX = param_2;
      puVar4 = extraout_EAX;
      for (uVar3 = (int)cVar1 & 0x3fffffff; puVar4 = puVar4 + 1, uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar4 = *param_1;
        param_1 = param_1 + 1;
      }
      for (iVar2 = 0; iVar2 != 0; iVar2 = iVar2 + -1) {
        *(undefined1 *)puVar4 = *(undefined1 *)param_1;
        param_1 = (undefined4 *)((int)param_1 + 1);
        puVar4 = (undefined4 *)((int)puVar4 + 1);
      }
      iVar2 = FUN_00736920(extraout_EAX);
      if (iVar2 == 0) {
        FUN_007360a0(extraout_EAX);
      }
    }
  }
  return;
}


