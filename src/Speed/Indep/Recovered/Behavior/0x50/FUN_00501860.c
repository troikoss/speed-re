// Original UG2 PC v1.2 0x501860, 98 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int __stdcall FUN_00501750(int);
#define DAT_0083845c (*(undefined4 *)0x0083845c)

int FUN_00501860(int param_1,int param_2)

{
  void *_this;
  short *psVar1;
  
  if ((((param_2 != 0) && (0 < param_1)) && (param_1 < 0x57)) &&
     (_this = (void *)(DAT_0083845c + -0x28 + param_1 * 0x28), _this != (void *)0x0)) {
    if (param_2 != 4) {
      psVar1 = (short *)FUN_00501750(_this,param_2);
      return (int)*psVar1;
    }
    switch(param_1) {
    case 0xb:
    case 0xc:
    case 0xd:
    case 0x22:
    case 0x4a:
    case 0x4b:
      return 400;
    case 0x16:
      return 1000;
    case 0x3e:
      return 100;
    }
  }
  return 0;
}


