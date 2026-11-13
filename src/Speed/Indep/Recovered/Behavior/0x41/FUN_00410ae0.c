// Original UG2 PC v1.2 0x410ae0, 109 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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

#define DAT_0088a334 (*(undefined4 *)0x0088a334)

int __fastcall FUN_00410ae0(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  short *psVar2;
  uint uVar3;
  
  iVar1 = 0;
  psVar2 = (short *)((int)_this + 4);
  while ((int)*psVar2 != (uint)*(ushort *)(param_1 + 10)) {
    iVar1 = iVar1 + 1;
    psVar2 = psVar2 + 1;
    if (1 < iVar1) {
      return 0;
    }
  }
  if (iVar1 == -1) {
    return 0;
  }
  uVar3 = (uint)(iVar1 == 0);
  iVar1 = (int)*(short *)((int)_this + uVar3 * 2 + 4);
  if (iVar1 < 0) {
    return *(short *)((int)_this + uVar3 * 2 + 10) * 0x38 + 0x8c;
  }
  return *(short *)((int)_this + uVar3 * 2 + 10) * 0x38 + 0x8c + (&DAT_0088a334)[iVar1];
}


