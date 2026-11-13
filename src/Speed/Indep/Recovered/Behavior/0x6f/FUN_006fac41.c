// Original UG2 PC v1.2 0x6fac41, 39 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_006faa2e();


undefined4 FUN_006fac41(undefined1 *param_1,int param_2)

{
  undefined4 local_8;
  
  local_8 = 0;
  if (param_2 != 0) {
    FUN_006faa2e(param_1,param_2,0,&local_8,(undefined4 *)0x0);
  }
  return local_8;
}


