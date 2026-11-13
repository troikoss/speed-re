// Original UG2 PC v1.2 0x490850, 142 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int hash;

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
int __cdecl FUN_0043bd50();
int __cdecl FUN_004901d0();
int __stdcall FUN_0048fe00(int);
#define DAT_0082db08 (*(undefined4 *)0x0082db08)

undefined1 __fastcall FUN_00490850(void *_this,int _edx_unused,void *param_1)

{
  int iVar1;
  uint hash;
  undefined1 uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  undefined1 local_5;
  
  local_5 = 0;
  uVar2 = 0;
  if ('\0' < *(char *)((int)_this + 0x19)) {
    iVar5 = 0;
    do {
      iVar1 = *(int *)((int)_this + 0x2c);
      uVar4 = *(uint *)(iVar1 + 4 + iVar5 * 8);
      hash = *(uint *)(iVar1 + iVar5 * 8);
      iVar1 = iVar1 + iVar5 * 8;
      if (*(void **)(uVar4 + 0x68) == param_1) {
        iVar3 = FUN_0048fe00(param_1,hash);
        if (iVar3 == 0) {
          uVar4 = FUN_004901d0(hash,0,0);
          if (uVar4 == 0) {
            uVar4 = DAT_0082db08;
            if (DAT_0082db08 == 0) {
                    /* WARNING: Subroutine does not return */
              FUN_0043bd50();
            }
          }
          else {
          }
        }
      }
      if (uVar4 != *(uint *)(iVar1 + 4)) {
        *(uint *)(iVar1 + 4) = uVar4;
        local_5 = 1;
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < *(char *)((int)_this + 0x19));
    uVar2 = local_5;
  }
  return uVar2;
}


