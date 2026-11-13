// Original UG2 PC v1.2 0x441f70, 48 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_007ec1fc (*(undefined4 *)0x007ec1fc)
#define PTR_DAT_007ec200 (*(undefined4 *)0x007ec200)
#define s_Uknown_007875a8 ((char *)0x007875a8)

char * FUN_00441f70(int param_1)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = 0;
  piVar1 = &DAT_007ec1fc;
  do {
    if (param_1 == *piVar1) {
      return (&PTR_DAT_007ec200)[iVar2 * 5];
    }
    piVar1 = piVar1 + 5;
    iVar2 = iVar2 + 1;
  } while ((int)piVar1 < 0x7ec274);
  return s_Uknown_007875a8;
}


