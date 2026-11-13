// Original UG2 PC v1.2 0x736480, 66 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
int __cdecl FUN_00736350();


undefined4 FUN_00736480(int param_1,int *param_2)

{
  int iVar1;
  int *local_4;
  
  iVar1 = FUN_00736350(param_1,&local_4);
  if (iVar1 == 0) {
    return 0;
  }
  *param_2 = ((uint)*(ushort *)(*local_4 + 0x10) * 2 + 3 & 0xfffffffc) + 0x18 + *local_4;
  return 1;
}


