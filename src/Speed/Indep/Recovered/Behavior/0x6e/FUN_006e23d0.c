// Original UG2 PC v1.2 0x6e23d0, 38 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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
int __cdecl FUN_006e6bf0();
int __cdecl FUN_006e7070();
int __cdecl FUN_006e70a0();
int __cdecl FUN_006e70d0();
int __cdecl FUN_006e75a0();


void __stdcall FUN_006e23d0(undefined2 *param_1)

{
  switch(*param_1) {
  case 0:
    FUN_006e75a0((int)param_1);
    return;
  case 0xf:
    FUN_006e6bf0((int)param_1);
    return;
  case 0x16:
    FUN_006e70d0((int)param_1);
    return;
  case 0x17:
    FUN_006e70a0((int)param_1);
    return;
  case 0x19:
    FUN_006e7070((int)param_1);
  }
  return;
}


