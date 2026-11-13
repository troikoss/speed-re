// Original UG2 PC v1.2 0x671152, 361 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __stdcall FUN_0068978d(int,int,int);
int __stdcall FUN_00689808(int,int);
int __stdcall FUN_00689b13(int,int,int);
int __stdcall FUN_00689b53(int,int,int);
int __stdcall FUN_00689bac(int,int,int);
int __stdcall FUN_00689d1b(int,int,int);
int __stdcall FUN_00689fc7(int,int,int);
int __stdcall FUN_0068a1d0(int,int);
int __stdcall FUN_0068a26c(int,int,int,int,int,int);
int __stdcall FUN_0068a320(int,int,int,int,int,int);
int __stdcall FUN_0068a3a7(int,int,int,int,int,int);
int __stdcall FUN_0068a44e(int,int,int,int,int,int);
int __stdcall FUN_0068a503(int,int,int,int,int,int);
int __stdcall FUN_0068a59f(int,int,int,int,int,int);
int __stdcall FUN_0068a627(int,int,int,int,int,int);
int __stdcall FUN_0068a683(int,int,int,int,int,int);
int __stdcall FUN_0068a6cb(int,int,int,int,int,int);
int __stdcall FUN_0068a71c(int,int,int,int);
int __stdcall FUN_0068a7cc(int);
int __stdcall FUN_0068ab60(int,int,int,int,int,int,int,int);
int __stdcall FUN_0068b0d8(int,int,int);
int __stdcall FUN_0068c07d(int,int,int,int,int,int);
int __stdcall FUN_0068c250(int,int,int,int,int,int);
int __stdcall FUN_0068c3d0(int,int,int,int,int,int);
int __stdcall FUN_0068c52c(int,int,int,int,int,int);
int __stdcall FUN_0068c63d(int,int,int,int,int,int);
int __stdcall FUN_0068c83d(int,int,int,int,int,int);
int __stdcall FUN_0068c9f1(int,int,int,int,int,int);
int __stdcall FUN_0068cce8(int,int,int);
int __stdcall FUN_0068cdbc(int,int,int);
int __stdcall FUN_0068ce7c(int,int,int);
int __stdcall FUN_0068cfb2(int,int);
int __stdcall FUN_0068d046(int,int);
int __stdcall FUN_0068d0e8(int,int,int,int);
int __stdcall FUN_0068d1a9(int,int,int,int);
int __stdcall FUN_0068d295(int,int,int);
int __stdcall FUN_0068d38d(int,int);
int __stdcall FUN_0068d465(int,int,int);
int __stdcall FUN_0068d531(int,int,int);
int __stdcall FUN_0068d625(int,int,int,int);
int __stdcall FUN_0068d6e4(int,int);


void FUN_00671152(undefined4 *param_1)

{
  param_1[2] = FUN_0068cdbc;
  param_1[0x23] = FUN_0068cdbc;
  *param_1 = FUN_00689b13;
  param_1[1] = FUN_0068b0d8;
  param_1[3] = FUN_0068ce7c;
  param_1[4] = FUN_00689fc7;
  param_1[5] = FUN_00689bac;
  param_1[6] = FUN_0068d465;
  param_1[9] = FUN_00689b53;
  param_1[10] = FUN_0068d531;
  param_1[7] = FUN_0068cfb2;
  param_1[8] = FUN_0068a1d0;
  param_1[0x16] = FUN_0068d046;
  param_1[0x11] = FUN_0068d0e8;
  param_1[0x15] = FUN_00689808;
  param_1[0x14] = FUN_0068978d;
  param_1[0x19] = FUN_0068d6e4;
  param_1[0x1a] = FUN_0068d625;
  param_1[0x17] = FUN_0068a71c;
  param_1[0x13] = FUN_00689d1b;
  param_1[0x32] = FUN_0068a44e;
  param_1[0x33] = FUN_0068a26c;
  param_1[0x34] = FUN_0068a627;
  param_1[0x35] = FUN_0068a503;
  param_1[0x36] = FUN_0068a3a7;
  param_1[0x37] = FUN_0068a683;
  param_1[0x12] = FUN_0068a59f;
  param_1[0x30] = FUN_0068a320;
  param_1[0x31] = FUN_0068a6cb;
  param_1[0xb] = FUN_0068a7cc;
  param_1[0x1c] = FUN_0068d295;
  param_1[0xe] = FUN_0068d1a9;
  param_1[0x1d] = FUN_0068d38d;
  param_1[0x3b] = FUN_0068cce8;
  param_1[0x3c] = FUN_0068c250;
  param_1[0x3d] = FUN_0068c3d0;
  param_1[0x3e] = FUN_0068c52c;
  param_1[0x3f] = FUN_0068c07d;
  param_1[0x40] = FUN_0068c9f1;
  param_1[0x41] = FUN_0068c83d;
  param_1[0x42] = FUN_0068c63d;
  param_1[0x45] = FUN_0068c63d;
  param_1[0x47] = FUN_0068ab60;
  return;
}


