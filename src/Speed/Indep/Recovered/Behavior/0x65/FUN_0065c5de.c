// Original UG2 PC v1.2 0x65c5de, 65 bytes. BEHAVIOUR-VERIFIED ONLY (34 clean trials, 6 both-fault):
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



int __fastcall FUN_0065c5de(void *_this, int _edx_unused, uint param_1, int _pad0)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = ~param_1;
  if ((*(int *)(uVar2 + 0x10) == 1) &&
     ((uint)*(ushort *)(uVar2 + 0x14) < *(uint *)((int)_this + 0xa0))) {
    iVar1 = *(int *)((int)_this + 0xa4) + (uint)*(ushort *)(uVar2 + 0x14) * 0xb4;
    if ((uint)*(ushort *)(uVar2 + 0x18) < *(uint *)(iVar1 + 0x34)) {
      return (uint)*(ushort *)(uVar2 + 0x18) * 0x38 + *(int *)(iVar1 + 0x38);
    }
  }
  return 0;
}


