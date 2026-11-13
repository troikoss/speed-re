// Original UG2 PC v1.2 0x742190, 57 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



uint FUN_00742190(char *param_1,uint param_2)

{
  char cVar1;
  
  if (((param_1 != (char *)0x0) && (' ' < *param_1)) && (*param_1 < '\x7f')) {
    param_2 = 0x20202020;
    while ((cVar1 = *param_1, ' ' < cVar1 && (cVar1 < '\x7f'))) {
      param_2 = param_2 << 8 | (int)cVar1;
      param_1 = param_1 + 1;
      if (0x20fffffe < (int)param_2) {
        return param_2;
      }
    }
  }
  return param_2;
}


