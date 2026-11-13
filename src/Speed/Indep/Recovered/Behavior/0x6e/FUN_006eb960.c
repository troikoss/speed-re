// Original UG2 PC v1.2 0x6eb960, 86 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



uint __fastcall FUN_006eb960(int param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar3 = (uint)*(byte *)(param_1 + 6);
  uVar4 = 1 << (*(byte *)(param_1 + 0x10) & 0x1f);
  uVar2 = (uint)*(ushort *)(param_1 + 4) % uVar4;
  iVar1 = uVar3 * 0x10 + 0x12 +
          param_1 + ((uVar4 + 1) * uVar3 * 3 + 1 & 0xfffffffe) * (*(ushort *)(param_1 + 4) / uVar4);
  if (0 < (int)uVar2) {
    iVar1 = iVar1 + (uVar2 + 1) * uVar3 * 3;
  }
  return *(byte *)(param_1 + 7) + 1 + iVar1 & 0xfffffffe;
}


