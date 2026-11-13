// Original UG2 PC v1.2 0x4436f0, 82 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __stdcall FUN_00435be0(int,int);
int __stdcall FUN_00455c80(int);


char __fastcall FUN_004436f0(void *_this,int _edx_unused,uint param_1,int param_2)

{
  char cVar1;
  
  cVar1 = FUN_00435be0(param_1,param_2);
  if (cVar1 == '\0') {
    *(undefined4 *)((int)_this + 0x1cd0) = 2;
    return '\0';
  }
  *(uint *)((int)_this + 0x1c58) = param_1;
  FUN_00455c80(_this,4);
  *(undefined4 *)((int)_this + 0x1cc8) = 300;
  return cVar1;
}


