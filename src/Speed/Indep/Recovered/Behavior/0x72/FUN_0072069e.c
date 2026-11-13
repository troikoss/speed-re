// Original UG2 PC v1.2 0x72069e, 46 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



undefined4 FUN_0072069e(byte *param_1)

{
  byte *pbVar1;
  int iVar2;
  
  pbVar1 = param_1 + *(short *)(param_1 + 2) * 4 + 8;
  pbVar1[0] = 0;
  pbVar1[1] = 0;
  pbVar1[2] = 0;
  pbVar1[3] = 0;
  iVar2 = *(int *)(param_1 + 4);
  if ((0 < iVar2) && (iVar2 <= (int)(uint)*param_1)) {
    *(undefined4 *)(param_1 + iVar2 * 4 + 8) = *(undefined4 *)(param_1 + 8);
    *(undefined2 *)(param_1 + 2) = *(undefined2 *)(param_1 + 4);
  }
  return *(undefined4 *)(param_1 + 0xc);
}


