// Original UG2 PC v1.2 0x541a10, 53 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
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
int __stdcall FUN_005287a0(int);
#define DAT_0085ba65 (*(undefined4 *)0x0085ba65)
#define DAT_0085ba64 (*(undefined4 *)0x0085ba64)
#define DAT_0085ef48 (*(undefined4 *)0x0085ef48)

void __fastcall FUN_00541a10(int param_1)

{
  *(undefined4 *)(param_1 + 0x16774) = 2;
  if ((*(int *)(param_1 + 0x156a8) == 1) && (DAT_0085ba65 != '\0')) {
    FUN_005287a0(&DAT_0085ba64,0);
    *(undefined4 *)(DAT_0085ef48 + 0x14) = 2;
  }
  return;
}


