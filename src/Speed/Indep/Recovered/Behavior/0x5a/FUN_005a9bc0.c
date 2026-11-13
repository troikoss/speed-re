// Original UG2 PC v1.2 0x5a9bc0, 53 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int __cdecl FUN_005a0cd0();
#define DAT_0086b490 (*(undefined4 *)0x0086b490)

void FUN_005a9bc0(void)

{
  undefined4 *puVar1;
  
  puVar1 = DAT_0086b490;
  if ((undefined4 **)DAT_0086b490 != &DAT_0086b490) {
    do {
      if ((puVar1 != (undefined4 *)0x0) && (puVar1 + -0x8c != (undefined4 *)0x0)) {
        (**(code **)puVar1[-0x8c])(1);
        puVar1 = DAT_0086b490;
      }
    } while ((undefined4 **)puVar1 != &DAT_0086b490);
  }
  FUN_005a0cd0();
  return;
}


