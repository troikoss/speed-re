// Original UG2 PC v1.2 0x512170, 175 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_00505450();
#define s_GenericDialog_Animate_LARGE_fng_00799e74 ((char *)0x00799e74)
#define s_GenericDialog_Animate_MED_fng_00799e54 ((char *)0x00799e54)
#define s_GenericDialog_Animate_SMALL_fng_00799e34 ((char *)0x00799e34)
#define s_GenericDialog_LARGE_fng_00790038 ((char *)0x00790038)
#define s_GenericDialog_MED_fng_0078f2f8 ((char *)0x0078f2f8)
#define s_GenericDialog_SMALL_fng_0078fa24 ((char *)0x0078fa24)
#define s_GenericDialog_ThreeButton_fng_00799e14 ((char *)0x00799e14)

undefined4 FUN_00512170(byte *param_1)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  char *pcVar4;
  
  uVar2 = FUN_00505450((char *)param_1);
  pcVar4 = s_GenericDialog_ThreeButton_fng_00799e14;
  uVar3 = 0xffffffff;
  bVar1 = 0x47;
  do {
    if (('`' < (char)bVar1) && ((char)bVar1 < '{')) {
      bVar1 = bVar1 - 0x20;
    }
    uVar3 = uVar3 * 0x21 + (uint)bVar1;
    bVar1 = pcVar4[1];
    pcVar4 = pcVar4 + 1;
  } while (bVar1 != 0);
  if (uVar2 != uVar3) {
    uVar3 = FUN_00505450(s_GenericDialog_Animate_SMALL_fng_00799e34);
    if (uVar2 != uVar3) {
      uVar3 = FUN_00505450(s_GenericDialog_SMALL_fng_0078fa24);
      if (uVar2 != uVar3) {
        uVar3 = FUN_00505450(s_GenericDialog_Animate_MED_fng_00799e54);
        if (uVar2 != uVar3) {
          uVar3 = FUN_00505450(s_GenericDialog_MED_fng_0078f2f8);
          if (uVar2 != uVar3) {
            uVar3 = FUN_00505450(s_GenericDialog_Animate_LARGE_fng_00799e74);
            if (uVar2 != uVar3) {
              uVar3 = FUN_00505450(s_GenericDialog_LARGE_fng_00790038);
              if (uVar2 != uVar3) {
                return 0;
              }
            }
          }
        }
      }
    }
  }
  return 1;
}


