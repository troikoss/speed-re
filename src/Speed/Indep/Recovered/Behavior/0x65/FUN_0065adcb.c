// Original UG2 PC v1.2 0x65adcb, 90 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



undefined4 __stdcall FUN_0065adcb(int param_1)

{
  uint *puVar1;
  ushort *puVar2;
  uint uVar3;
  
  *(byte *)(param_1 + 0x119) = *(byte *)(param_1 + 0x119) & 0xfe;
  if (*(int *)(param_1 + 0x10c) != 0) {
    uVar3 = 0;
    if (*(int *)(param_1 + 0x10c) != 0) {
      do {
        puVar2 = (ushort *)(*(int *)(param_1 + 0x108) + uVar3 * 4);
        puVar1 = (uint *)((uint)puVar2[1] * 100 +
                          *(int *)((uint)*puVar2 * 100 + 4 + *(int *)(param_1 + 0x2c)) + 0x30);
        *puVar1 = *puVar1 & 0xfffffffd;
        uVar3 = uVar3 + 1;
      } while (uVar3 < *(uint *)(param_1 + 0x10c));
    }
    *(undefined4 *)(param_1 + 0x10c) = 0;
  }
  return 0;
}


