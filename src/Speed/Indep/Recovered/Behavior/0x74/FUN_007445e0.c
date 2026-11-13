// Original UG2 PC v1.2 0x7445e0, 49 bytes. BEHAVIOUR-VERIFIED ONLY (27 clean trials, 13 both-fault):
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
int __cdecl FUN_00755090();


undefined4 FUN_007445e0(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  if (((param_2 < 0) || (0x23 < param_2)) ||
     (iVar1 = *(int *)(param_1 + 0x1160 + param_2 * 0x14), iVar1 == 0)) {
    return 0;
  }
  uVar2 = FUN_00755090(iVar1,param_3);
  return uVar2;
}


