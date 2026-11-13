// Original UG2 PC v1.2 0x4af780, 76 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00558790();
int __cdecl FUN_0055dda0();
#define s_GenericDialog_Small_fng_00790d74 ((char *)0x00790d74)

void __stdcall FUN_004af780(char *param_1, int param_2, int _pad0, int _pad1, int _pad2)

{
  void *_this;
  int textParameter;
  
  if (param_2 == 0xc407210) {
    textParameter = -0x6922561a;
    FUN_0055dda0(0x96dda9e6,param_1);
    FUN_00558790(_this,param_1,s_GenericDialog_Small_fng_00790d74,0xf770,0x61c,0x34dc1bec,0x1fab5998,
                 0x1fab5998,1,0xb10ae56f,textParameter);
  }
  return;
}


