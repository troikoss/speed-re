// Original UG2 PC v1.2 0x741570, 86 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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
int __cdecl FUN_007414a0();
int __cdecl FUN_0075c784();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_0078f6f4 (*(undefined4 *)0x0078f6f4)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_00741570(byte *param_1,int param_2,undefined4 param_3)

{
  char local_104 [256];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  FUN_0075c784(local_104,&DAT_0078f6f4,param_2);
  FUN_007414a0((char *)param_1,local_104);
  return;
}


