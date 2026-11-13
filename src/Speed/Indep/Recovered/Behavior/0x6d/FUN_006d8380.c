// Original UG2 PC v1.2 0x6d8380, 48 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



uint __fastcall FUN_006d8380(void *_this,int _edx_unused,char *param_1)

{
  char *pcVar1;
  char cVar2;
  uint uVar3;
  
  uVar3 = 0;
  cVar2 = *param_1;
  while (cVar2 != '\0') {
    pcVar1 = param_1 + 1;
    uVar3 = uVar3 ^ uVar3 << 5 ^ (int)cVar2;
    param_1 = param_1 + 1;
    cVar2 = *pcVar1;
  }
  return uVar3 % *(uint *)((int)_this + 4);
}


