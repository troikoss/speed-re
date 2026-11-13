// Original UG2 PC v1.2 0x721c4d, 285 bytes. BEHAVIOUR-VERIFIED ONLY (37 clean trials, 3 both-fault):
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
int __cdecl FUN_00721b48();
int __cdecl FUN_00721f99();
int __cdecl FUN_00721fa7();
int __cdecl FUN_00723e1e();
int __cdecl FUN_00727181();
int __cdecl FUN_00727669();
int __cdecl FUN_0072768f();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b44c4 (*(undefined4 *)0x008b44c4)
#define DAT_008b4670 (*(undefined4 *)0x008b4670)
#define DAT_008b460c (*(undefined4 *)0x008b460c)
#define DAT_008b466c (*(undefined4 *)0x008b466c)

undefined4 FUN_00721c4d(int param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined2 local_dc [102];
  int local_10;
  int local_c;
  undefined2 *local_8;
  
  if (DAT_008b4608 == '\0') {
    return 0xfffffff6;
  }
  if (param_1 == -1) {
    iVar3 = 0;
    if (DAT_008b44c4 != 0) {
      do {
        FUN_00721c4d(iVar3);
        iVar3 = iVar3 + 1;
      } while (iVar3 < (int)(uint)DAT_008b44c4);
    }
  }
  else {
    uVar1 = FUN_0072768f(param_1);
    if (uVar1 != 0) {
      return 0xfffffff8;
    }
    FUN_00721f99();
    iVar4 = param_1 * 8;
    iVar3 = *(int *)(iVar4 + DAT_008b4670);
    local_8 = (undefined2 *)0x0;
    if (0 < DAT_008b460c) {
      local_10 = 0;
      do {
        if (*(short *)(DAT_008b466c + local_10 + 0x12) == param_1) {
          FUN_00723e1e(*(uint *)(DAT_008b466c + local_10));
        }
        local_8 = (undefined2 *)((int)local_8 + 1);
        local_10 = local_10 + 0x90;
      } while ((int)local_8 < (int)DAT_008b460c);
    }
    iVar2 = 0;
    if (*(short *)(iVar3 + 6) != 0) {
      do {
        local_c = FUN_00727669(iVar3,iVar2);
        if (local_c != 0) {
          local_c = local_c + 4;
          do {
            local_8 = FUN_00727181(&local_c,local_dc);
            FUN_00721b48((int)local_dc,2);
          } while (local_8 != (undefined2 *)0x0);
        }
        iVar2 = iVar2 + 1;
      } while (iVar2 < (int)(uint)*(ushort *)(iVar3 + 6));
    }
    *(undefined4 *)(iVar4 + DAT_008b4670) = 0;
    *(undefined1 *)(iVar4 + 4 + DAT_008b4670) = 0;
    FUN_00721fa7();
  }
  return 0;
}


