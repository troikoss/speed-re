// Original UG2 PC v1.2 0x52d170, 308 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0052cf60();
#define s_UI_CHoosePaintCategory_fng_0079b92c ((char *)0x0079b92c)
#define s_UI_PerformanceTuning_Master_fng_00799bb4 ((char *)0x00799bb4)

int __stdcall FUN_0052d170(uint _this)

{
  int iVar1;
  bool bVar2;
  
  if (_this < 0x40001) {
    if (_this == 0x40000) {
      return -0x3ae64040;
    }
    if (_this < 0x101) {
      if (_this == 0x100) {
        return 0xc407210;
      }
      switch(_this) {
      case 4:
      case 8:
      case 0x10:
      case 0x20:
      case 0x40:
      case 0x80:
        goto switchD_0052d1a1_caseD_4;
      default:
        return 0;
      }
    }
    if (_this < 0x1001) {
      if (_this == 0x1000) {
        return -0x3ae64040;
      }
      if (_this == 0x200) {
        iVar1 = FUN_0052cf60(s_UI_CHoosePaintCategory_fng_0079b92c);
        return (-(uint)((char)iVar1 != '\0') & 0x33ff0c5c) + 0x911ab364;
      }
      if (_this == 0x400) {
        return -0x6ee54c9c;
      }
      if (_this == 0x800) {
        iVar1 = FUN_0052cf60(s_UI_PerformanceTuning_Master_fng_00799bb4);
        return ((char)iVar1 != '\0') + 0xc519bfbf;
      }
      return 0;
    }
    if (_this == 0x2000) {
      return -0x3ae64040;
    }
    bVar2 = _this == 0x4000;
  }
  else if (_this < 0x2000001) {
    if (_this == 0x2000000) {
switchD_0052d1a1_caseD_4:
      return 0x406415e3;
    }
    if (_this < 0x400001) {
      if (_this == 0x400000) {
        return -0x3ae6403d;
      }
      if (((_this == 0x80000) || (_this == 0x100000)) || (_this == 0x200000)) {
        return -0x3ac65b50;
      }
      return 0;
    }
    if (_this == 0x800000) {
      return -0x3ae64040;
    }
    bVar2 = _this == 0x1000000;
  }
  else {
    if (_this < 0x10000001) {
      if (_this == 0x10000000) {
        return -0x3ae64040;
      }
      if (_this == 0x4000000) {
        return 0x406415e3;
      }
      if (_this == 0x8000000) {
        return 0x406415e3;
      }
      return 0;
    }
    if (_this == 0x20000000) {
      return -0x3ae64040;
    }
    bVar2 = _this == 0x40000000;
  }
  if (bVar2) {
    return -0x3ae64040;
  }
  return 0;
}


