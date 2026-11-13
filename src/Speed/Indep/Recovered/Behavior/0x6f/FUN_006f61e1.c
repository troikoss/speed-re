// Original UG2 PC v1.2 0x6f61e1, 124 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008ad458 (*(undefined4 *)0x008ad458)
#define DAT_008ad450 (*(undefined4 *)0x008ad450)
#define DAT_008ad454 (*(undefined4 *)0x008ad454)
#define s_AthlonMP_007c2dfc ((char *)0x007c2dfc)
#define s_AthlonXP_007c2df0 ((char *)0x007c2df0)
#define s_Athlon_007c2de8 ((char *)0x007c2de8)
#define s_Cyrix_007c2de0 ((char *)0x007c2de0)
#define s_Pentium_III_007c2e14 ((char *)0x007c2e14)
#define s_Pentium_II_007c2e08 ((char *)0x007c2e08)
#define s_Pentium_IV_007c2e20 ((char *)0x007c2e20)
#define s_Unknown_0078fe78 ((char *)0x0078fe78)
#define s_WinChip_007c2dd8 ((char *)0x007c2dd8)

char * FUN_006f61e1(void)

{
  char *pcVar1;
  
  if ((DAT_008ad458 & 0x1000000) == 0) {
    if ((DAT_008ad458 & 0x2000000) == 0) {
      if ((DAT_008ad458 & 0x4000000) != 0) {
        return s_Cyrix_007c2de0;
      }
      pcVar1 = s_WinChip_007c2dd8;
      if ((DAT_008ad458 & 0x8000000) == 0) {
        pcVar1 = s_Unknown_0078fe78;
      }
      return pcVar1;
    }
    if ((DAT_008ad450 & 0x2000000) == 0) {
      return s_Athlon_007c2de8;
    }
    if (DAT_008ad454 < 2) {
      return s_AthlonXP_007c2df0;
    }
    return s_AthlonMP_007c2dfc;
  }
  if ((DAT_008ad450 & 0x2000000) == 0) {
    return s_Pentium_II_007c2e08;
  }
  if ((DAT_008ad450 & 0x4000000) == 0) {
    return s_Pentium_III_007c2e14;
  }
  return s_Pentium_IV_007c2e20;
}


