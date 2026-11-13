// Original UG2 PC v1.2 0x755e80, 194 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



uint FUN_00755e80(int param_1,int param_2)

{
  int iVar1;
  
  if (param_2 == 0x646f6e65) {
    if (*(int *)(param_1 + 0x108) != 7) {
      return (uint)(*(int *)(param_1 + 0x108) == 6);
    }
  }
  else if (param_2 == 0x64617461) {
    iVar1 = *(int *)(param_1 + 0x108);
    if (iVar1 != 7) {
      if ((iVar1 == 5) || (iVar1 == 6)) {
        return 1;
      }
      return 0;
    }
  }
  else {
    if (param_2 == 0x6d6f6465) {
      return *(uint *)(param_1 + 0x10c);
    }
    iVar1 = *(int *)(param_1 + 0x108);
    if (iVar1 != 7) {
      if ((iVar1 != 5) && (iVar1 != 6)) {
        return 0xfffffffe;
      }
      if (param_2 == 0x68656164) {
        return *(uint *)(param_1 + 0x11c);
      }
      if (param_2 == 0x626f6479) {
        return *(uint *)(param_1 + 0x120);
      }
      if (param_2 == 0x64617465) {
        return *(uint *)(param_1 + 0x114);
      }
      if (param_2 == 0x636f6465) {
        return *(uint *)(param_1 + 0x110);
      }
      return 0;
    }
  }
  return 0xffffffff;
}


