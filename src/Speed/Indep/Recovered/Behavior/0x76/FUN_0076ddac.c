// Original UG2 PC v1.2 0x76ddac, 94 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_0076dd8b();


/* Library Function - Single Match
    ___add_12
   
   Library: Visual Studio 2003 Release */

void FUN_0076ddac(uint *param_1,uint *param_2)

{
  int iVar1;
  
  iVar1 = FUN_0076dd8b(*param_1,*param_2,param_1);
  if (iVar1 != 0) {
    iVar1 = FUN_0076dd8b(param_1[1],1,param_1 + 1);
    if (iVar1 != 0) {
      param_1[2] = param_1[2] + 1;
    }
  }
  iVar1 = FUN_0076dd8b(param_1[1],param_2[1],param_1 + 1);
  if (iVar1 != 0) {
    param_1[2] = param_1[2] + 1;
  }
  FUN_0076dd8b(param_1[2],param_2[2],param_1 + 2);
  return;
}


