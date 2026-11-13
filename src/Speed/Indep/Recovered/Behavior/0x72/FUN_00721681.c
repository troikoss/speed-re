// Original UG2 PC v1.2 0x721681, 69 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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
int __cdecl FUN_0072163d();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_00818084 (*(undefined4 *)0x00818084)
#define DAT_008b4028 (*(undefined4 *)0x008b4028)
#define DAT_008b402c (*(undefined4 *)0x008b402c)

undefined4 FUN_00721681(int param_1)

{
  undefined4 uVar1;
  char local_5;
  
  FUN_0072163d(param_1,&local_5);
  if (local_5 == '\0') {
    uVar1 = 0xfffffffb;
  }
  else {
    if (DAT_008b4608 == '\0') {
      DAT_00818084 = param_1;
    }
    else {
      DAT_008b4028 = param_1;
      DAT_008b402c = 1;
    }
    uVar1 = 0;
  }
  return uVar1;
}


