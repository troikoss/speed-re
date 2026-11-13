// Original UG2 PC v1.2 0x70423c, 61 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0070410d();
int __cdecl FUN_007041a2();
#define DAT_008b3e70 (*(undefined4 *)0x008b3e70)

void FUN_0070423c(int param_1,int param_2,int param_3,int *param_4,int *param_5)

{
  FUN_0070410d((byte *)(param_1 - param_3),0x8b3e50,param_3,1,0xb,8,param_4);
  FUN_007041a2((int *)&DAT_008b3e70,param_2,8,8,8,8,param_5);
  return;
}


