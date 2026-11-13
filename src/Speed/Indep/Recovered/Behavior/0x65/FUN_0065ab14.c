// Original UG2 PC v1.2 0x65ab14, 465 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_0065aaaa();


int __fastcall FUN_0065ab14(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  
  if (*(int *)((int)_this + 0xb4) == 0) {
    iVar1 = *(int *)(*(int *)((int)_this + 0xb0) + *(int *)(param_1 + 0x14) * 4);
    if (*(int *)(iVar1 + 0x34) != 0) {
      puVar4 = *(undefined4 **)(*(int *)(iVar1 + 0x34) + 0x18);
      if ((*(byte *)((int)_this + 0x119) & 1) == 0) {
        piVar3 = puVar4 + 7;
        do {
          puVar4 = (undefined4 *)*piVar3;
          *puVar4 = 1;
          *(undefined4 *)
           ((uint)*(ushort *)((int)puVar4 + 0x16) * 100 +
           *(int *)((uint)*(ushort *)(puVar4 + 5) * 100 + 4 + *(int *)((int)_this + 0x2c))) = 1;
          piVar3 = puVar4 + 7;
        } while (*piVar3 != *(int *)(iVar1 + 0x34));
      }
      else {
        do {
          puVar4 = (undefined4 *)puVar4[7];
          *puVar4 = 1;
          puVar5 = (undefined4 *)
                   ((uint)*(ushort *)((int)puVar4 + 0x16) * 100 +
                   *(int *)((uint)*(ushort *)(puVar4 + 5) * 100 + 4 + *(int *)((int)_this + 0x2c)));
          *puVar5 = 1;
          if ((*(byte *)(puVar5 + 0xc) & 2) == 0) {
            if ((*(uint *)((int)_this + 0x110) <= *(uint *)((int)_this + 0x10c)) &&
               (iVar2 = FUN_0065aaaa((int)_this), iVar2 < 0)) {
              return iVar2;
            }
            *(undefined2 *)(*(int *)((int)_this + 0x108) + *(int *)((int)_this + 0x10c) * 4) =
                 *(undefined2 *)(puVar4 + 5);
            *(undefined2 *)(*(int *)((int)_this + 0x108) + 2 + *(int *)((int)_this + 0x10c) * 4) =
                 *(undefined2 *)((int)puVar4 + 0x16);
            puVar5[0xc] = puVar5[0xc] | 2;
            *(int *)((int)_this + 0x10c) = *(int *)((int)_this + 0x10c) + 1;
          }
        } while (puVar4[7] != *(int *)(iVar1 + 0x34));
      }
    }
  }
  else if (*(uint *)(param_1 + 0x14) < *(uint *)((int)_this + 0xac)) {
    *(undefined1 *)(*(uint *)(param_1 + 0x14) + *(int *)((int)_this + 0xbc)) = 1;
    piVar3 = *(int **)(*(int *)((int)_this + 0xb8) + *(int *)(param_1 + 0x14) * 4);
    if (piVar3 != (int *)0x0) {
      if ((*(byte *)((int)_this + 0x119) & 1) == 0) {
        for (; (undefined4 *)*piVar3 != (undefined4 *)0x0; piVar3 = piVar3 + 1) {
          *(undefined4 *)*piVar3 = 1;
          **(undefined4 **)(*piVar3 + 8) = 1;
        }
      }
      else {
        for (; puVar4 = (undefined4 *)*piVar3, puVar4 != (undefined4 *)0x0; piVar3 = piVar3 + 1) {
          puVar5 = (undefined4 *)puVar4[2];
          *puVar4 = 1;
          *puVar5 = 1;
          if ((*(byte *)(puVar5 + 0xc) & 2) == 0) {
            if ((*(uint *)((int)_this + 0x110) <= *(uint *)((int)_this + 0x10c)) &&
               (iVar1 = FUN_0065aaaa((int)_this), iVar1 < 0)) {
              return iVar1;
            }
            *(undefined2 *)(*(int *)((int)_this + 0x108) + *(int *)((int)_this + 0x10c) * 4) =
                 *(undefined2 *)(puVar4 + 5);
            *(undefined2 *)(*(int *)((int)_this + 0x108) + 2 + *(int *)((int)_this + 0x10c) * 4) =
                 *(undefined2 *)((int)puVar4 + 0x16);
            puVar5[0xc] = puVar5[0xc] | 2;
            *(int *)((int)_this + 0x10c) = *(int *)((int)_this + 0x10c) + 1;
          }
        }
      }
    }
  }
  return 0;
}


