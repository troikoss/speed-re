// Original UG2 PC v1.2 0x7223c2, 98 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int b;

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



int FUN_007223c2(ushort *param_1)

{
  char cVar1;
  int iStack_8;
  
  cVar1 = *(char *)((int)param_1 + 3);
  iStack_8 = 0;
  if (cVar1 == '\n') {
    iStack_8 = 0x88;
  }
  else if (cVar1 == '\x04') {
    iStack_8 = 0x33;
  }
  else if (cVar1 == '\x16') {
    iStack_8 = 0x66;
  }
  else if (cVar1 == '\t') {
    iStack_8 = 0x100;
  }
  else if (cVar1 == '\b') {
    iStack_8 = 0x200;
  }
  else if ((cVar1 == '\x10') || (cVar1 == '\x17')) {
    return (uint)(byte)param_1[1] << 3;
  }
  return (int)((uint)*param_1 * (uint)(byte)param_1[1] * iStack_8) >> 8;
}


