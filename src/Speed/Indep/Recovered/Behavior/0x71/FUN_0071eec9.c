// Original UG2 PC v1.2 0x71eec9, 71 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0071e9e7();
int __cdecl FUN_0071ea86();


void FUN_0071eec9(int param_1,undefined8 *param_2,int param_3,short *param_4,short *param_5)

{
  undefined8 local_104;
  undefined1 local_fc [248];
  
  FUN_0071e9e7(param_1 - param_3,&local_104,param_3,1,0xb,8,param_4);
  FUN_0071ea86((int)local_fc,param_2,8,8,8,8,param_5);
  return;
}


