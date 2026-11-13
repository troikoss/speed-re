// Original UG2 PC v1.2 0x49b980, 83 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int textParameter;

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
int __cdecl FUN_0050d150();
int __cdecl FUN_00558790();
#define s_GenericDialog_SMALL_fng_0078fa24 ((char *)0x0078fa24)

void __stdcall FUN_0049b980(char *param_1,int param_2)

{
  void *_this;
  char *textParameter;
  
  if ((param_2 == 0xc407210) || (param_2 == 0x406415e3)) {
    textParameter = param_1;
    FUN_0050d150(param_1,1,0);
    FUN_00558790(_this,param_1,s_GenericDialog_SMALL_fng_0078fa24,0xf770,0x61c,0xd05fc3a3,0x34dc1bcf,
                 0x34dc1bcf,0,0x2bbcca6b,(int)textParameter);
  }
  return;
}


