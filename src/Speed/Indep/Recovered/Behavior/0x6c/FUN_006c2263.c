// Original UG2 PC v1.2 0x6c2263, 70 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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



undefined4 __stdcall FUN_006c2263(int param_1,int param_2,int param_3)

{
  uint uVar1;
  
  if (*(int *)(param_2 + 0x28) != param_3) {
    uVar1 = 0;
    *(int *)(param_2 + 0x28) = param_3;
    if (*(int *)(param_2 + 0x1c) != 0) {
      do {
        FUN_006c2263(param_1,*(int *)(param_1 + *(int *)(*(int *)(param_2 + 0x20) + uVar1 * 4) * 4),
                     param_3);
        uVar1 = uVar1 + 1;
      } while (uVar1 < *(uint *)(param_2 + 0x1c));
    }
  }
  return 0;
}


