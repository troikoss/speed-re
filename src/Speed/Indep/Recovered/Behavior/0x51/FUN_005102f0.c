// Original UG2 PC v1.2 0x5102f0, 83 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 __stdcall FUN_005102f0(uint param_1, int _pad0, int _pad1, int _pad2, int _pad3)

{
  undefined4 in_stack_00000014;
  
  if (param_1 < 0x9120409f) {
    if ((((param_1 != 0x9120409e) && (param_1 != 0x72619778)) && (param_1 != 0x911ab364)) &&
       (param_1 != 0x911c0a4b)) {
      return 0;
    }
  }
  else if (param_1 != 0xb5971bf1) {
    if (param_1 == 0xc519bfc0) {
      return 10;
    }
    if (param_1 == 0xc519bfc3) {
      return 2;
    }
    return 0;
  }
  return in_stack_00000014;
}


