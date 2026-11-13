// Original UG2 PC v1.2 0x4a0df0, 146 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int dialogParentFNGName;
typedef int textFormat;
typedef int textParameter;

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
int __cdecl FUN_0049e810();
int __cdecl FUN_004ffa80();
int __cdecl FUN_00540a60();
int __cdecl FUN_00555eb0();
int __cdecl FUN_00558540();
int __cdecl FUN_00582b60();
int __cdecl FUN_0073ea30();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define s_UI_OLX_FindResults_fng_007901e8 ((char *)0x007901e8)

void __fastcall FUN_004a0df0(void *_this,int _edx_unused,undefined4 param_1,int param_2)

{
  char *dialogParentFNGName;
  int extraout_EAX;
  char *textFormat;
  void *this_00;
  int textParameter;
  
  if (*(int *)((int)_this + 0x4c) != 0) {
    FUN_00540a60(*(int *)((int)_this + 0x4c));
    *(undefined4 *)((int)_this + 0x4c) = 0;
  }
  if (param_2 != 0) {
    FUN_0049e810(*(char **)((int)_this + 4),param_2);
    return;
  }
  FUN_00582b60();
  FUN_0073ea30();
  if (extraout_EAX == 0) {
    dialogParentFNGName = *(char **)((int)_this + 4);
    textParameter = -0x74df81e6;
    textFormat = FUN_004ffa80(this_00,0x8b207e1a);
    FUN_00558540(dialogParentFNGName,&DAT_00783fe5,0x639,0xffffffff,0xffffffff,textFormat,
                 textParameter);
    return;
  }
  FUN_00555eb0((byte *)s_UI_OLX_FindResults_fng_007901e8,*(byte **)((int)_this + 4),0xff,0);
  return;
}


