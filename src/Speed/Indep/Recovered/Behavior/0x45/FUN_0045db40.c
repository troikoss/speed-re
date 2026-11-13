// Original UG2 PC v1.2 0x45db40, 75 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



void __fastcall FUN_0045db40(int param_1)

{
  int iVar1;
  
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  *(undefined4 *)(param_1 + 0x10) = 0;
  if (*(char *)(param_1 + 0x70) != '\0') {
    iVar1 = *(int *)(param_1 + 0x78);
    if (5 < iVar1) {
      if (iVar1 < 8) {
        *(undefined4 *)(param_1 + 8) = 1;
        *(undefined4 *)(param_1 + 0x10) = 1;
        return;
      }
      if (iVar1 == 0x14) {
        *(undefined4 *)(param_1 + 0xc) = 1;
        *(undefined4 *)(param_1 + 0x10) = 1;
        return;
      }
    }
    *(undefined4 *)(param_1 + 4) = 1;
    *(undefined4 *)(param_1 + 0x10) = 1;
  }
  return;
}


