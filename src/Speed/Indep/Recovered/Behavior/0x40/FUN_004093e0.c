// Original UG2 PC v1.2 0x4093e0, 192 bytes. BEHAVIOUR-VERIFIED ONLY (32 clean trials, 8 both-fault):
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
#define sRam00000034 (*(short *)0x00000034)

int __fastcall FUN_004093e0(void *_this,int _edx_unused,int param_1,char param_2,int param_3,int param_4)

{
  short sVar1;
  int iVar2;
  
  sVar1 = *(short *)((int)_this + param_1 * 2 + 4);
  if (sVar1 != -1) {
    if (*(char *)(param_4 + (param_1 + 0x68 + param_3 * 2) * 2 + (int)_this) != '\0') {
      iVar2 = *(int *)_this;
      if (param_2 != '\0') {
        if (iVar2 < 0) {
          return sVar1 * 0x38 + 0x8c;
        }
        return sVar1 * 0x38 + 0x8c + (&DAT_0088a334)[iVar2];
      }
      if (param_1 != 0) {
        if (iVar2 < 0) {
          return sRam00000034 * 0x38 + 0x54;
        }
        return *(short *)((&DAT_0088a334)[iVar2] + 0x34) * 0x38 + 0x54 + (&DAT_0088a334)[iVar2];
      }
      if (iVar2 < 0) {
        return 0x8c;
      }
      return (&DAT_0088a334)[iVar2] + 0x8c;
    }
  }
  return 0;
}


