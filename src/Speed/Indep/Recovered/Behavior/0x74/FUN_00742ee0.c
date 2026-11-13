// Original UG2 PC v1.2 0x742ee0, 115 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined4 FUN_00742ee0(int param_1,byte *param_2)

{
  byte bVar1;
  int iVar2;
  
  iVar2 = 0;
  while( true ) {
    *(undefined1 *)(iVar2 + 4 + param_1) = 0;
    bVar1 = *param_2;
    while (('/' < (char)bVar1 && ((char)bVar1 < ':'))) {
      param_2 = param_2 + 1;
      *(byte *)(iVar2 + 4 + param_1) = *(char *)(iVar2 + 4 + param_1) * '\n' + (bVar1 & 0xf);
      bVar1 = *param_2;
    }
    if ((int)(char)*param_2 != ((2 < iVar2) - 1 & 0x2e)) break;
    iVar2 = iVar2 + 1;
    param_2 = param_2 + 1;
    if (3 < iVar2) {
      return 0;
    }
  }
  *(undefined1 *)(param_1 + 5) = 0;
  *(undefined1 *)(param_1 + 4) = 0;
  *(undefined1 *)(param_1 + 7) = 0;
  *(undefined1 *)(param_1 + 6) = 0;
  return 0xffffffff;
}


