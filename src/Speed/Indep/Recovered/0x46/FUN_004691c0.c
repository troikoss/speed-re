// Original UG2 PC v1.2 0x4691c0, 60 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.
typedef int doPrint;
typedef int format;
typedef int param;

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
int __cdecl FUN_0043fbc0();
int __cdecl FUN_004400d0();
#define DAT_0082c738 (*(undefined4 *)0x0082c738)
#define DAT_007fb724 (*(undefined4 *)0x007fb724)
#define s___6d__00789cf8 ((char *)0x00789cf8)

void FUN_004691c0(int doPrint,char *format,void *param)

{
  char local_10 [16];
  
  if (doPrint != 0) {
    FUN_0043fbc0(&DAT_0082c738,format,&param);
    FUN_004400d0(local_10,s___6d__00789cf8,DAT_007fb724);
  }
  return;
}


