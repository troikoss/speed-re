// Original UG2 PC v1.2 0x6c9acd, 112 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



undefined4 __fastcall FUN_006c9acd(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  uint uVar2;
  uint *puVar3;
  int *piVar4;
  int *piVar5;
  uint uVar6;
  uint uVar7;
  
  iVar1 = *(int *)(*(int *)((int)_this + 0x14) + param_1 * 4);
  uVar6 = *(uint *)(iVar1 + 0x54);
  if (((uVar6 != 0xffffffff) && ((*(byte *)((int)_this + 0x6f) & 0x20) == 0)) &&
     (uVar2 = *(uint *)(iVar1 + 0x58), uVar6 <= uVar2)) {
    piVar4 = (int *)(*(int *)((int)_this + 0x18) + uVar6 * 4);
    do {
      puVar3 = (uint *)*piVar4;
      if ((*puVar3 & 0xf0000000) == 0x60000000) {
        uVar7 = 0;
        if (puVar3[1] != 0) {
          piVar5 = (int *)puVar3[2];
          do {
            if (*piVar5 == param_1) {
              return 1;
            }
            uVar7 = uVar7 + 1;
            piVar5 = piVar5 + 1;
          } while (uVar7 < puVar3[1]);
        }
      }
      uVar6 = uVar6 + 1;
      piVar4 = piVar4 + 1;
    } while (uVar6 <= uVar2);
  }
  return 0;
}


