// Original UG2 PC v1.2 0x49a050, 178 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.

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
int __cdecl FUN_0051cda0();
int __cdecl FUN_0052cea0();
int __cdecl FUN_00537c00();
#define s_Highlight_0078f228 ((char *)0x0078f228)
#define s_UnHighlight_0078f21c ((char *)0x0078f21c)

void __fastcall FUN_0049a050(void *_this,int _edx_unused,int param_1)

{
  if (*(int *)((int)_this + 0x54) != param_1) {
    FUN_00537c00(*(byte **)((int)_this + 4),
                 *(undefined4 *)((int)_this + *(int *)((int)_this + 0x54) * 4 + 0x58),
                 (byte *)s_UnHighlight_0078f21c,'\x01');
    FUN_00537c00(*(byte **)((int)_this + 4),
                 *(undefined4 *)((int)_this + *(int *)((int)_this + 0x54) * 4 + 0x68),
                 (byte *)s_UnHighlight_0078f21c,'\x01');
    FUN_0051cda0(*(byte **)((int)_this + 4),
                 *(int *)((int)_this + *(int *)((int)_this + 0x54) * 4 + 0x58),'\0');
    *(int *)((int)_this + 0x54) = param_1;
    FUN_0051cda0(*(byte **)((int)_this + 4),*(int *)((int)_this + param_1 * 4 + 0x58),'\x01');
    FUN_0052cea0(*(byte **)((int)_this + 4),
                 *(int *)((int)_this + *(int *)((int)_this + 0x54) * 4 + 0x58));
    FUN_00537c00(*(byte **)((int)_this + 4),
                 *(undefined4 *)((int)_this + *(int *)((int)_this + 0x54) * 4 + 0x58),
                 (byte *)s_Highlight_0078f228,'\x01');
    FUN_00537c00(*(byte **)((int)_this + 4),
                 *(undefined4 *)((int)_this + *(int *)((int)_this + 0x54) * 4 + 0x68),
                 (byte *)s_Highlight_0078f228,'\x01');
  }
  return;
}


