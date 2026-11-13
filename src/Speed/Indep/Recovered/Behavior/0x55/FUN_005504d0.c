// Original UG2 PC v1.2 0x5504d0, 155 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00495460();
int __stdcall FUN_0054e6e0(int,int);
#define DAT_00838560 (*(undefined4 *)0x00838560)
#define DAT_008384d8 (*(undefined4 *)0x008384d8)

void __fastcall FUN_005504d0(int param_1)

{
  switch(DAT_00838560) {
  case 1:
  case 2:
    FUN_00495460(*(char **)(param_1 + 4),0xdc0eba9c,0x3a9ccf4e);
    FUN_0054e6e0(DAT_008384d8,*(int *)(param_1 + 0x20),*(char **)(param_1 + 4));
    return;
  case 3:
    FUN_00495460(*(char **)(param_1 + 4),0xdc0eba9c,0xded7ca79);
    FUN_0054e6e0(DAT_008384d8,*(int *)(param_1 + 0x20),*(char **)(param_1 + 4));
    return;
  case 4:
    FUN_00495460(*(char **)(param_1 + 4),0xdc0eba9c,0x6a0b254c);
    FUN_0054e6e0(DAT_008384d8,0x409,*(char **)(param_1 + 4));
  }
  return;
}


