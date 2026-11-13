// Original UG2 PC v1.2 0x73f680, 69 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_0073e880();
int __cdecl FUN_0073edd0();


undefined4 FUN_0073f680(undefined4 param_1,int param_2)

{
  uint uVar1;
  uint local_4;
  
  uVar1 = FUN_0073e880(param_2,1,&local_4);
  if (0x7fffffff < uVar1) {
    uVar1 = FUN_0073e880(param_2,0,&local_4);
    if ((int)uVar1 < 0) {
      return 0xffffffff;
    }
  }
  FUN_0073edd0(param_2,uVar1);
  return 0;
}


