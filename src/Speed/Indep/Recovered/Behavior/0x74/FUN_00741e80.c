// Original UG2 PC v1.2 0x741e80, 63 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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
int __cdecl FUN_00741270();
int __cdecl FUN_007414a0();


void FUN_00741e80(byte *param_1,int param_2,byte *param_3,byte *param_4)

{
  byte *pbVar1;
  
  pbVar1 = (byte *)FUN_007414a0((char *)param_4,(char *)param_3);
  if (pbVar1 == (byte *)0x0) {
    return;
  }
  for (; (pbVar1 != param_4 && (0x20 < pbVar1[-1])); pbVar1 = pbVar1 + -1) {
  }
  FUN_00741270(param_1,param_2,pbVar1);
  return;
}


