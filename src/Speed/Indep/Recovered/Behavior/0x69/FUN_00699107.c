// Original UG2 PC v1.2 0x699107, 359 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __fastcall FUN_00699107(uint *param_1)

{
  uint uVar1;
  bool bVar2;
  
  uVar1 = *param_1 & 0xfff00000;
  if (uVar1 < 0x73800001) {
    if (uVar1 == 0x73800000) {
      return 1;
    }
    if (uVar1 < 0x20900001) {
      if (uVar1 == 0x20900000) {
        return 1;
      }
      if (uVar1 < 0x11400001) {
        if (uVar1 == 0x11400000) {
          return 1;
        }
        if (uVar1 == 0x10f00000) {
          return 1;
        }
        if (uVar1 == 0x11100000) {
          return 1;
        }
        if (uVar1 == 0x11200000) {
          return 1;
        }
        bVar2 = uVar1 == 0x11300000;
      }
      else {
        if (uVar1 == 0x11500000) {
          return 1;
        }
        if (uVar1 == 0x20700000) {
          return 1;
        }
        bVar2 = uVar1 == 0x20800000;
      }
    }
    else if (uVar1 < 0x73400001) {
      if (uVar1 == 0x73400000) {
        return 1;
      }
      if (uVar1 == 0x73000000) {
        return 1;
      }
      if (uVar1 == 0x73100000) {
        return 1;
      }
      if (uVar1 == 0x73200000) {
        return 1;
      }
      bVar2 = uVar1 == 0x73300000;
    }
    else {
      if (uVar1 == 0x73500000) {
        return 1;
      }
      if (uVar1 == 0x73600000) {
        return 1;
      }
      bVar2 = uVar1 == 0x73700000;
    }
  }
  else if (uVar1 < 0x74100001) {
    if (uVar1 == 0x74100000) {
      return 1;
    }
    if (uVar1 < 0x73d00001) {
      if (uVar1 == 0x73d00000) {
        return 1;
      }
      if (uVar1 == 0x73900000) {
        return 1;
      }
      if (uVar1 == 0x73a00000) {
        return 1;
      }
      if (uVar1 == 0x73b00000) {
        return 1;
      }
      bVar2 = uVar1 == 0x73c00000;
    }
    else {
      if (uVar1 == 0x73e00000) {
        return 1;
      }
      if (uVar1 == 0x73f00000) {
        return 1;
      }
      bVar2 = uVar1 == 0x74000000;
    }
  }
  else if (uVar1 < 0x74600001) {
    if (uVar1 == 0x74600000) {
      return 1;
    }
    if (uVar1 == 0x74200000) {
      return 1;
    }
    if (uVar1 == 0x74300000) {
      return 1;
    }
    if (uVar1 == 0x74400000) {
      return 1;
    }
    bVar2 = uVar1 == 0x74500000;
  }
  else {
    if (uVar1 == 0x74700000) {
      return 1;
    }
    if (uVar1 == 0x74a00000) {
      return 1;
    }
    bVar2 = uVar1 == 0x74b00000;
  }
  if (bVar2) {
    return 1;
  }
  return 0;
}


