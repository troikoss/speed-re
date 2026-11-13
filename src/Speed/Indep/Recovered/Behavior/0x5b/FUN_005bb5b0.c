// Original UG2 PC v1.2 0x5bb5b0, 328 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

#define DAT_007ff064 (*(undefined4 *)0x007ff064)
#define DAT_007ff074 (*(undefined4 *)0x007ff074)
#define DAT_007ff060 (*(undefined4 *)0x007ff060)
#define DAT_007ff070 (*(undefined4 *)0x007ff070)
#define DAT_007ff068 (*(undefined4 *)0x007ff068)
#define DAT_007ff078 (*(undefined4 *)0x007ff078)
#define DAT_007ff080 (*(undefined4 *)0x007ff080)
#define DAT_007ff084 (*(undefined4 *)0x007ff084)
#define DAT_007ff06c (*(undefined4 *)0x007ff06c)
#define DAT_007ff07c (*(undefined4 *)0x007ff07c)

void __fastcall FUN_005bb5b0(int param_1)

{
  uint uVar1;
  
  uVar1 = *(uint *)(param_1 + 0xc);
  if (uVar1 < 0x89f23a15) {
    if (uVar1 == 0x89f23a14) {
LAB_005bb65e:
      *(undefined4 *)(param_1 + 0x50) = DAT_007ff064;
      *(undefined4 *)(param_1 + 0x54) = DAT_007ff074;
      return;
    }
    if (uVar1 < 0x3ed70c44) {
      if (uVar1 == 0x3ed70c43) {
LAB_005bb63a:
        *(undefined4 *)(param_1 + 0x50) = DAT_007ff060;
        *(undefined4 *)(param_1 + 0x54) = DAT_007ff070;
        return;
      }
      if (uVar1 < 0x23e6a3f5) {
        if (uVar1 != 0x23e6a3f4) {
          if (uVar1 == 0x12c9453c) goto LAB_005bb63a;
          if (uVar1 != 0x2388dd82) {
            return;
          }
        }
      }
      else if (uVar1 != 0x23e6b396) {
        if (uVar1 != 0x33a26cb6) {
          return;
        }
        goto LAB_005bb60c;
      }
    }
    else if (uVar1 < 0x8050ca40) {
      if (uVar1 == 0x8050ca3f) goto LAB_005bb6e6;
      if (uVar1 == 0x471a1dca) goto LAB_005bb63a;
      if (uVar1 != 0x6cf41096) {
        return;
      }
    }
    else if (uVar1 != 0x8062e2c0) {
      if (uVar1 != 0x89946400) {
        return;
      }
      goto LAB_005bb65e;
    }
  }
  else if (uVar1 < 0x8cbe3498) {
    if (uVar1 != 0x8cbe3497) {
      if (uVar1 < 0x8cb694ee) {
        if (uVar1 != 0x8cb694ed) {
          if (uVar1 == 0x8a616c4c) goto LAB_005bb6e6;
          if (uVar1 != 0x8cb61dcd) {
            return;
          }
        }
      }
      else if ((uVar1 != 0x8cb95e6b) && (uVar1 != 0x8cbc02ec)) {
        return;
      }
    }
  }
  else {
    if (0xc9d2b801 < uVar1) {
      if ((uVar1 != 0xd68ea0c1) && (uVar1 != 0xd6ec76d5)) {
        return;
      }
LAB_005bb6e6:
      *(undefined4 *)(param_1 + 0x50) = DAT_007ff068;
      *(undefined4 *)(param_1 + 0x54) = DAT_007ff078;
      return;
    }
    if (uVar1 != 0xc9d2b801) {
      if (uVar1 == 0xa6348ee3) {
LAB_005bb60c:
        *(undefined4 *)(param_1 + 0x50) = DAT_007ff080;
        *(undefined4 *)(param_1 + 0x54) = DAT_007ff084;
        return;
      }
      if (uVar1 != 0xb3645f81) {
        return;
      }
    }
  }
  *(undefined4 *)(param_1 + 0x50) = DAT_007ff06c;
  *(undefined4 *)(param_1 + 0x54) = DAT_007ff07c;
  return;
}


