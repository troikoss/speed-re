// Original UG2 PC v1.2 0x735a60, 63 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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

#define DAT_008b517c (*(undefined4 *)0x008b517c)
#define DAT_008b5174 (*(undefined4 *)0x008b5174)

void FUN_00735a60(int param_1,int param_2)

{
  int iVar1;
  
  DAT_008b517c = param_1;
  DAT_008b5174 = param_2;
  if ((param_2 != 0) && (iVar1 = 0, 0 < param_1)) {
    do {
      *(undefined4 *)(DAT_008b5174 + iVar1 * 8) = 0;
      *(undefined4 *)(DAT_008b5174 + 4 + iVar1 * 8) = 0;
      iVar1 = iVar1 + 1;
    } while (iVar1 < DAT_008b517c);
  }
  return;
}


