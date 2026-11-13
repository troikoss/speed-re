// Original UG2 PC v1.2 0x43e430, 102 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



int __fastcall FUN_0043e430(int param_1)

{
  int iVar1;
  byte *pbVar2;
  
  if (*(int *)(param_1 + 0x160) < *(int *)(param_1 + 0x154)) {
    pbVar2 = (byte *)(*(int *)(param_1 + 0x15c) + *(int *)(param_1 + 0x160) * 0xc);
    if (pbVar2 != (byte *)0x0) {
      if (pbVar2[1] == 0) {
        iVar1 = *(int *)(param_1 + 0x160) + (uint)*(ushort *)(pbVar2 + 2);
        *(int *)(param_1 + 0x160) = iVar1;
        if (*(int *)(param_1 + 0x154) <= iVar1) {
          return 0;
        }
        pbVar2 = (byte *)(*(int *)(param_1 + 0x15c) + iVar1 * 0xc);
        if (pbVar2 == (byte *)0x0) {
          return 0;
        }
      }
      *(int *)(param_1 + 0x160) = *(int *)(param_1 + 0x160) + 1;
      return *(int *)((*pbVar2 + 1) * 0x54 + param_1) + *(int *)(pbVar2 + 4);
    }
  }
  return 0;
}


