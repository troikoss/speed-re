// Original UG2 PC v1.2 0x4aed30, 73 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_004022c0();
int __cdecl FUN_00404800();
int __cdecl FUN_0050e2d0();
int __cdecl FUN_00537980();
#define DAT_00850078 (*(undefined4 *)0x00850078)
#define s_UI_QRCarSelect_fng_0078f354 ((char *)0x0078f354)

void __stdcall FUN_004aed30(char *param_1, int param_2, int _pad0, int _pad1, int _pad2)

{
  char cVar1;
  
  if (param_2 == 0xc407210) {
    DAT_00850078 = 0x20;
    FUN_0050e2d0();
    cVar1 = FUN_00404800();
    if (cVar1 != '\0') {
      FUN_004022c0();
      return;
    }
    FUN_00537980(param_1,s_UI_QRCarSelect_fng_0078f354,0);
  }
  return;
}


