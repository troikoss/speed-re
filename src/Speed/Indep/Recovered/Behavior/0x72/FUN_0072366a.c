// Original UG2 PC v1.2 0x72366a, 47 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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
int __cdecl FUN_00721f99();
int __cdecl FUN_00721fa7();
int __cdecl FUN_00727e6b();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b43ec (*(undefined4 *)0x008b43ec)

undefined4 FUN_0072366a(int param_1)

{
  if (DAT_008b4608 == '\0') {
    return 0xfffffff6;
  }
  FUN_00721f99();
  FUN_00727e6b();
  (&DAT_008b43ec)[param_1] = 0;
  FUN_00721fa7();
  return 0;
}


