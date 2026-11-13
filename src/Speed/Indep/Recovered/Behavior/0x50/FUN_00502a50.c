// Original UG2 PC v1.2 0x502a50, 75 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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



int __fastcall FUN_00502a50(void *_this,int _edx_unused,int param_1,int param_2,int param_3)

{
  char *pcVar1;
  int iVar2;
  
  iVar2 = 0;
  pcVar1 = (char *)((int)_this + 2);
  while (((*(short *)(pcVar1 + -2) != param_1 || (pcVar1[1] != param_2)) || (*pcVar1 != param_3))) {
    iVar2 = iVar2 + 1;
    pcVar1 = pcVar1 + 8;
    if (0x13f < iVar2) {
      return 0;
    }
  }
  return (int)(void *)((int)_this + iVar2 * 8);
}


