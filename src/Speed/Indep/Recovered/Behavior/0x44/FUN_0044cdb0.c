// Original UG2 PC v1.2 0x44cdb0, 67 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __stdcall FUN_0048f760(int);
#define PTR_FUN_00787cf0 (*(code *)0x00787cf0)
#define DAT_00883dfc (*(undefined4 *)0x00883dfc)

void __fastcall FUN_0044cdb0(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_00787cf0;
  if (param_1[5] != 0) {
    param_1[5] = 0;
    param_1[9] = *(undefined4 *)(param_1[7] + 0x280);
    param_1[10] = *(undefined4 *)(param_1[7] + 0x284);
    FUN_0048f760((int)param_1);
  }
  if (param_1[3] == 2) {
    DAT_00883dfc = DAT_00883dfc + -1;
  }
  return;
}


