// Original UG2 PC v1.2 0x73bad0, 141 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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
int __cdecl FUN_0073c6b0();
#define DAT_008b7bc4 (*(undefined4 *)0x008b7bc4)
#define DAT_008b7bb0 (*(undefined4 *)0x008b7bb0)

int FUN_0073bad0(int param_1)

{
  int iVar1;
  int extraout_EAX;
  uint local_1c [7];
  
  if (param_1 == 0) {
    return 1;
  }
  *(undefined4 *)(param_1 + 0x100) = 2;
  if (*(uint *)(param_1 + 0x4c) != 0) {
    iVar1 = 4 - (uint)(DAT_008b7bc4 < *(uint *)(param_1 + 0x4c));
    *(int *)(param_1 + 0x100) = iVar1;
    return iVar1;
  }
  if (-1 < *(int *)(param_1 + 0x34)) {
    *(undefined4 *)(param_1 + 0x100) = 7;
    FUN_0073c6b0(param_1,local_1c);
    if ((-1 < extraout_EAX) &&
       ((uint)param_1 <= (uint)(*(int *)(DAT_008b7bb0 + 0xc) + *(int *)(param_1 + 0x24)))) {
      *(undefined4 *)(param_1 + 0x100) = 5;
    }
  }
  return *(int *)(param_1 + 0x100);
}


