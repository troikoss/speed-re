// Original UG2 PC v1.2 0x6e88f0, 114 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



uint __fastcall FUN_006e88f0(int param_1)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  bVar1 = *(byte *)(param_1 + 0x10);
  uVar4 = (uint)*(ushort *)(param_1 + 0xe);
  uVar2 = param_1 + (((1 << (bVar1 & 0x1f)) + 1) * uVar4 * 3 + 1 & 0xfffffffe) *
                    (uint)(*(ushort *)(param_1 + 0xc) >> (bVar1 & 0x1f)) + 0x14 + uVar4 * 0x24;
  uVar3 = 0x7fffffffU >> (0x1f - bVar1 & 0x1f) & (uint)*(ushort *)(param_1 + 0xc);
  if (uVar3 != 0) {
    uVar2 = (uVar3 + 1) * uVar4 * 3 + 1 + uVar2 & 0xfffffffe;
  }
  if (*(ushort *)(param_1 + 0xe) == 0) {
    uVar2 = uVar2 + 1 & 0xfffffffe;
  }
  return uVar2;
}


