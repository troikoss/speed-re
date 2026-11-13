// Original UG2 PC v1.2 0x43e620, 43 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



undefined4 FUN_0043e620(char *param_1)

{
  char *pcVar1;
  char *pcVar2;
  
  pcVar2 = param_1;
  if (*param_1 != '\0') {
    do {
      pcVar1 = pcVar2 + 1;
      pcVar2 = pcVar2 + 1;
    } while (*pcVar1 != '\0');
    if (*param_1 != '\0') {
      do {
        pcVar2 = param_1 + 1;
        param_1 = param_1 + 1;
      } while (*pcVar2 != '\0');
    }
  }
  return 0;
}


