// Original UG2 PC v1.2 0x736350, 75 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

#define DAT_008b4fc8 (*(undefined4 *)0x008b4fc8)

undefined4 FUN_00736350(int param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  while (((iVar1 = (&DAT_008b4fc8)[iVar2 * 2], iVar1 == 0 ||
          (*(char *)(iVar1 + 9) != *(char *)(param_1 + 2))) ||
         (*(char *)(iVar1 + 8) != *(char *)(param_1 + 3)))) {
    iVar2 = iVar2 + 1;
    if (7 < iVar2) {
      return 0;
    }
  }
  *param_2 = &DAT_008b4fc8 + iVar2 * 2;
  return 1;
}


