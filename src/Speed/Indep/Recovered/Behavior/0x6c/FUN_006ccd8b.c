// Original UG2 PC v1.2 0x6ccd8b, 139 bytes. BEHAVIOUR-VERIFIED ONLY (39 clean trials, 1 both-fault):
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
int __cdecl FUN_0064307c();
#define DAT_00786e70 (*(undefined4 *)0x00786e70)

undefined4
__stdcall FUN_006ccd8b(undefined4 param_1,int param_2,short param_3,int param_4,undefined1 param_5,int param_6
            )

{
  int iVar1;
  undefined1 local_24 [16];
  undefined4 local_14;
  int local_10;
  
  if (param_2 == -1) {
    if (((*(int *)(param_6 + 0x100) != 0) &&
        (iVar1 = *(int *)(*(int *)(param_6 + 0x100) + 0x3c), iVar1 != 0)) &&
       (*(int *)(iVar1 + 4) == 0xd)) {
      param_1 = *(undefined4 *)(iVar1 + 0x40);
      param_2 = *(int *)(iVar1 + 0x44);
      if (param_2 != -1) goto LAB_006ccdc8;
    }
    param_2 = 0;
  }
LAB_006ccdc8:
  local_14 = param_1;
  if ((param_3 == 2) || (param_3 == 6)) {
    local_10 = param_2;
    FUN_0064307c(*(void **)(param_6 + 0xc4),(int)local_24,param_4 + 5000,&DAT_00786e70,param_5);
    *(undefined4 *)(param_6 + 0xd4) = 1;
  }
  return 0;
}


