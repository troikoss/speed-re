// Original UG2 PC v1.2 0x4ac7d0, 121 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_0043dbb0();
#define DAT_00790b60 (*(undefined4 *)0x00790b60)
#define DAT_00790b50 (*(undefined4 *)0x00790b50)
#define s_LONG_GO_00790b48 ((char *)0x00790b48)
#define s_MID_GO_00790b58 ((char *)0x00790b58)
#define s_SHORT_00790b70 ((char *)0x00790b70)
#define s_SHORT_GO_00790b64 ((char *)0x00790b64)

void __fastcall FUN_004ac7d0(void *_this,int _edx_unused,char *param_1,char *param_2,char *param_3)

{
  int iVar1;
  int iVar2;
  
  iVar1 = FUN_0043dbb0(param_2);
  iVar2 = FUN_0043dbb0(param_3);
  if (iVar1 < iVar2) {
    iVar1 = iVar2;
  }
  iVar2 = FUN_0043dbb0(param_1);
  if (iVar1 < iVar2) {
    iVar1 = iVar2;
  }
  if (iVar1 < 0x12) {
    *(char **)((int)_this + 0x58) = s_SHORT_00790b70;
    *(char **)((int)_this + 0x5c) = s_SHORT_GO_00790b64;
    return;
  }
  if (iVar1 < 0x24) {
    *(undefined **)((int)_this + 0x58) = &DAT_00790b60;
    *(char **)((int)_this + 0x5c) = s_MID_GO_00790b58;
    return;
  }
  *(undefined **)((int)_this + 0x58) = &DAT_00790b50;
  *(char **)((int)_this + 0x5c) = s_LONG_GO_00790b48;
  return;
}


