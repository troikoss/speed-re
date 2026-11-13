// Original UG2 PC v1.2 0x502ff0, 71 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define s_PC_OL_Lobby_fng_0078f320 ((char *)0x0078f320)
#define s_UI_Main_fng_0078f530 ((char *)0x0078f530)
#define s_UI_QRModeSelect_fng_0078f398 ((char *)0x0078f398)

char * __fastcall FUN_00502ff0(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  char *pcVar2;
  
  pcVar2 = s_UI_Main_fng_0078f530;
  if (param_1 == 0) {
    iVar1 = *(int *)((int)_this + 0x156a8);
    if ((iVar1 == 1) || (iVar1 == 2)) {
      return s_UI_Main_fng_0078f530;
    }
    if (iVar1 == 4) {
      return s_UI_QRModeSelect_fng_0078f398;
    }
    if ((iVar1 == 8) || (iVar1 == 0x200)) {
      pcVar2 = s_PC_OL_Lobby_fng_0078f320;
    }
  }
  return pcVar2;
}


