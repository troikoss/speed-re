// Original UG2 PC v1.2 0x570fa0, 48 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int arg0;
typedef int filename;

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
int __cdecl FUN_0043dc00();


void __fastcall FUN_00570fa0(void *_this,int _edx_unused,int arg0,char *filename)

{
  int iVar1;
  char *pcVar2;
  
  *(int *)((int)_this + 0xc4) = arg0;
  pcVar2 = (char *)((int)_this + 0x94);
  for (iVar1 = 0xc; iVar1 != 0; iVar1 = iVar1 + -1) {
    pcVar2[0] = '\0';
    pcVar2[1] = '\0';
    pcVar2[2] = '\0';
    pcVar2[3] = '\0';
    pcVar2 = pcVar2 + 4;
  }
  FUN_0043dc00((char *)((int)_this + 0x94),filename,0x2f);
  return;
}


