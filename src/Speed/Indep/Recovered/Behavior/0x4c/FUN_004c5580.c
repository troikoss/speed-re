// Original UG2 PC v1.2 0x4c5580, 272 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_004c4f50();
int __cdecl FUN_004c5080();
int __cdecl FUN_0051de60();
int __cdecl FUN_00537c00();
int __cdecl FUN_0055dda0();
#define DAT_008651ac (*(undefined4 *)0x008651ac)
#define DAT_00836495 (*(undefined4 *)0x00836495)
#define _DAT_00784478 (*(undefined4 *)0x00784478)
#define _DAT_00784268 (*(undefined4 *)0x00784268)
#define s_Chyron_fng_00795984 ((char *)0x00795984)
#define s_FadeIn_0078f970 ((char *)0x0078f970)
#define s_UG_LS_IntroFMV_fng_0078ea74 ((char *)0x0078ea74)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004c5580(void *_this, int _edx_unused, uint param_1, undefined4 param_2, uint param_3, int _pad0)

{
  int iVar1;
  
  if (param_1 < 0xb5af2462) {
    if (param_1 != 0xb5af2461) {
      if (param_1 == 0x35f8620b) {
        *(int *)((int)_this + 0x4c) = DAT_008651ac;
        FUN_0055dda0(0x69696969,s_Chyron_fng_00795984);
        return;
      }
      if (param_1 != 0x406415e3) {
        return;
      }
    }
    if (DAT_00836495 != '\0') {
      FUN_0051de60(*(void **)((int)_this + 4),9);
      FUN_004c4f50(*(char **)((int)_this + 4),param_3);
      return;
    }
  }
  else if (param_1 == 0xc98356ba) {
    if (_DAT_00784478 < (float)(DAT_008651ac - *(int *)((int)_this + 0x50)) * _DAT_00784268) {
      FUN_004c5080(*(int *)((int)_this + 4),s_UG_LS_IntroFMV_fng_0078ea74);
      return;
    }
    if ((((DAT_00836495 == '\0') && (iVar1 = *(int *)((int)_this + 0x4c), iVar1 != 0)) &&
        (iVar1 != 0x7fffffff)) && (16000 < DAT_008651ac - iVar1)) {
      DAT_00836495 = '\x01';
      FUN_00537c00(*(byte **)((int)_this + 4),0x2ddf58c,(byte *)s_FadeIn_0078f970,'\x01');
      FUN_00537c00(*(byte **)((int)_this + 4),0x4c46f86a,(byte *)s_FadeIn_0078f970,'\x01');
    }
  }
  return;
}


