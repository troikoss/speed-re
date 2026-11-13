// Original UG2 PC v1.2 0x642b5d, 106 bytes. BEHAVIOUR-VERIFIED ONLY (31 clean trials, 9 both-fault):
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



undefined4 __fastcall FUN_00642b5d(void *_this,int _edx_unused,char *param_1,int param_2,undefined4 param_3,undefined4 param_4,int param_5,
            int param_6)

{
  char cVar1;
  char *pcVar2;
  
  if ((param_5 != 0) && (param_6 != 0)) {
    if (param_2 == -1) {
      if (param_1 == (char *)0x0) {
        param_2 = 0;
      }
      else {
        pcVar2 = param_1;
        do {
          cVar1 = *pcVar2;
          pcVar2 = pcVar2 + 1;
        } while (cVar1 != '\0');
        param_2 = (int)pcVar2 - (int)(param_1 + 1);
      }
    }
    if ((param_1 != (char *)0x0) || (param_2 == 0)) {
      *(char **)_this = param_1;
      *(undefined4 *)((int)_this + 0x18) = param_3;
      *(undefined4 *)((int)_this + 0x1c) = param_4;
      *(int *)((int)_this + 0x2c) = param_5;
      *(char **)((int)_this + 4) = param_1 + param_2;
      *(int *)((int)_this + 0x30) = param_6;
      return 0;
    }
  }
  return 0x80004005;
}


