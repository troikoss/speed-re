// Original UG2 PC v1.2 0x721de6, 45 bytes. BEHAVIOUR-VERIFIED ONLY (27 clean trials, 13 both-fault):
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

#define DAT_008b4058 (*(undefined4 *)0x008b4058)
#define DAT_008b4060 (*(undefined4 *)0x008b4060)
#define DAT_008b405c (*(undefined4 *)0x008b405c)

void FUN_00721de6(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  if (DAT_008b4058 != DAT_008b4060) {
    DAT_008b4060 = DAT_008b4058;
    puVar2 = DAT_008b405c;
    while (puVar2 != (undefined4 *)0x0) {
      puVar1 = (undefined4 *)*puVar2;
      (*(code *)puVar2[2])(puVar2[3]);
      puVar2 = puVar1;
    }
  }
  return;
}


