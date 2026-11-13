// Original UG2 PC v1.2 0x46ca10, 73 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004691c0();
int __cdecl FUN_007227cb();
int __cdecl FUN_0075c6da();
#define s_Deleting_Stream_ID__d_m_queuedOb_0078a990 ((char *)0x0078a990)

void __fastcall FUN_0046ca10(int param_1)

{
  if (*(int *)(param_1 + 0x48) != 0) {
    FUN_007227cb(*(int *)(*(int *)(param_1 + 0x48) + 0x18));
  }
  if ((*(int *)(param_1 + 0x44) != 0) && (*(int *)(*(int *)(param_1 + 0x44) + 8) == 7)) {
    FUN_004691c0(1,s_Deleting_Stream_ID__d_m_queuedOb_0078a990,(void *)0x7);
    FUN_0075c6da(*(void **)(param_1 + 0x44));
    *(undefined4 *)(param_1 + 0x44) = 0;
  }
  *(undefined1 *)(param_1 + 0x4c) = 0;
  return;
}


