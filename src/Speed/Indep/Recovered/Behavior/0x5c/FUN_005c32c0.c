// Original UG2 PC v1.2 0x5c32c0, 259 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_004400d0();
int __cdecl FUN_0057cac0();
int __cdecl FUN_005c2ed0();
#define DAT_00870838 (*(undefined4 *)0x00870838)
#define DAT_007fee5c (*(undefined4 *)0x007fee5c)
#define DAT_007fee60 (*(undefined4 *)0x007fee60)
#define DAT_00870830 (*(undefined4 *)0x00870830)
#define DAT_007fee64 (*(undefined4 *)0x007fee64)
#define DAT_00870840 (*(undefined4 *)0x00870840)
#define DAT_00870834 (*(undefined4 *)0x00870834)
#define DAT_0087083c (*(undefined4 *)0x0087083c)
#define s_StreamAnim_06d_tga_007a3c38 ((char *)0x007a3c38)
#define s__Screenshot_03d_tga_007a3c23 ((char *)0x007a3c23)

void FUN_005c32c0(void)

{
  int iVar1;
  char local_80 [128];
  
  if (DAT_00870838 == 2) {
    if (DAT_007fee5c < 1) {
      DAT_007fee5c = 1;
    }
    if (DAT_007fee60 < 1) {
      DAT_007fee60 = 1;
    }
    DAT_00870830 = DAT_00870830 + -1;
    if (DAT_00870830 < 1) {
      DAT_007fee64 = DAT_007fee64 + 1;
      DAT_00870830 = DAT_007fee60;
      FUN_004400d0(local_80,s_StreamAnim_06d_tga_007a3c38,DAT_007fee64);
      FUN_005c2ed0();
    }
    if (DAT_007fee64 == DAT_007fee5c + -1) {
      DAT_00870840 = 0;
    }
  }
  else {
    DAT_007fee64 = -1;
    DAT_00870830 = 0;
  }
  if (DAT_00870838 == 1) {
    do {
      FUN_004400d0(local_80,s__Screenshot_03d_tga_007a3c23 + 1,DAT_00870834);
      DAT_00870834 = DAT_00870834 + 1;
      iVar1 = FUN_0057cac0(local_80);
    } while (iVar1 != 0);
    FUN_005c2ed0();
    DAT_0087083c = 0;
  }
  return;
}


