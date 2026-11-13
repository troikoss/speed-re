// Original UG2 PC v1.2 0x723138, 156 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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
int __cdecl FUN_00721f99();
int __cdecl FUN_00721fa7();
int __cdecl FUN_00727e6b();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b44de (*(undefined4 *)0x008b44de)
#define DAT_008b43ec (*(undefined4 *)0x008b43ec)

int FUN_00723138(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4,int param_5)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  FUN_00721f99();
  iVar3 = 0;
  if (DAT_008b44de != 0) {
    do {
      if ((&DAT_008b43ec)[iVar3] == 0) {
        iVar1 = FUN_00727e6b();
        if (iVar1 < 0) {
          iVar3 = -6;
        }
        else {
          iVar1 = FUN_00727e6b();
          puVar2 = (undefined4 *)(param_4 + iVar1);
          iVar1 = FUN_00727e6b();
          puVar2[0x15] = param_1;
          puVar2[0x16] = param_2;
          *(short *)(puVar2 + 0x10) = (short)((param_5 - iVar1) - 0x9cU >> 5);
          puVar2[0x14] = param_4;
          puVar2[0x17] = param_3;
          *puVar2 = 0xffffffff;
          (&DAT_008b43ec)[iVar3] = puVar2;
        }
        goto LAB_0072316f;
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < (int)(uint)DAT_008b44de);
  }
  iVar3 = -9;
LAB_0072316f:
  FUN_00721fa7();
  return iVar3;
}


