// Original UG2 PC v1.2 0x582190, 155 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_004ff970();
int __cdecl FUN_00570a30();
#define s_14705_007a10c4 ((char *)0x007a10c4)
#define s_EAD08004144_007a1020 ((char *)0x007a1020)
#define s_EAE08004144_007a1038 ((char *)0x007a1038)
#define s_EAF08004144_007a102c ((char *)0x007a102c)
#define s_EAH08004144_007a0ffc ((char *)0x007a0ffc)
#define s_EAI08004144_007a1014 ((char *)0x007a1014)
#define s_EAN08004144_007a0ff0 ((char *)0x007a0ff0)
#define s_EAS08004144_007a1008 ((char *)0x007a1008)
#define s_EMW_4139_007a10ac ((char *)0x007a10ac)
#define s_SKU_CHINA_007a1054 ((char *)0x007a1054)
#define s_SKU_FRANCE_007a108c ((char *)0x007a108c)
#define s_SKU_GERMANY_007a1080 ((char *)0x007a1080)
#define s_SKU_ITALY_007a1074 ((char *)0x007a1074)
#define s_SKU_KOREA_007a10b8 ((char *)0x007a10b8)
#define s_SKU_ROA_007a1060 ((char *)0x007a1060)
#define s_SKU_SPAIN_007a1068 ((char *)0x007a1068)
#define s_SKU_TAIWAN_007a10a0 ((char *)0x007a10a0)
#define s_SKU_THAILAND_007a1044 ((char *)0x007a1044)
#define s_SKU_UK_007a1098 ((char *)0x007a1098)

char * FUN_00582190(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00570a30();
  switch(iVar1) {
  case 0:
    return s_14705_007a10c4;
  case 1:
    uVar2 = FUN_004ff970();
    switch(uVar2) {
    case 0:
      return s_EAE08004144_007a1038;
    case 1:
      return s_EAF08004144_007a102c;
    case 2:
      return s_EAD08004144_007a1020;
    case 3:
      return s_EAI08004144_007a1014;
    case 4:
      return s_EAS08004144_007a1008;
    case 5:
      return s_EAH08004144_007a0ffc;
    case 6:
    case 7:
      return s_EAN08004144_007a0ff0;
    }
    break;
  case 2:
    return s_SKU_KOREA_007a10b8;
  case 3:
    return s_EMW_4139_007a10ac;
  case 4:
    return s_SKU_TAIWAN_007a10a0;
  case 5:
    return s_SKU_UK_007a1098;
  case 6:
    return s_SKU_FRANCE_007a108c;
  case 7:
    return s_SKU_GERMANY_007a1080;
  case 8:
    return s_SKU_ITALY_007a1074;
  case 9:
    return s_SKU_SPAIN_007a1068;
  case 10:
    return s_SKU_ROA_007a1060;
  case 0xb:
    return s_SKU_CHINA_007a1054;
  case 0xc:
    return s_SKU_THAILAND_007a1044;
  }
  return (char *)0x0;
}


