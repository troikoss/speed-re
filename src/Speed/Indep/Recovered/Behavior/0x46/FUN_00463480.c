// Original UG2 PC v1.2 0x463480, 90 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
int __cdecl FUN_00440540();
int __cdecl FUN_00721a63();
#define DAT_0082b08c (*(undefined4 *)0x0082b08c)
#define DAT_0082b090 (*(undefined4 *)0x0082b090)
#define DAT_0082af60 (*(undefined4 *)0x0082af60)

void FUN_00463480(void)

{
  undefined4 *puVar1;
  
  if (DAT_0082b08c != (int *)0x0) {
    FUN_00721a63(DAT_0082b08c);
    DAT_0082b08c = (int *)0x0;
  }
  if (DAT_0082b090 != (int *)0x0) {
    FUN_00721a63(DAT_0082b090);
    DAT_0082b090 = (int *)0x0;
  }
  puVar1 = &DAT_0082af60;
  do {
    if ((void *)*puVar1 != (void *)0x0) {
      FUN_00440540((void *)*puVar1);
    }
    *puVar1 = 0;
    puVar1 = puVar1 + 1;
  } while ((int)puVar1 < 0x82af90);
  return;
}


