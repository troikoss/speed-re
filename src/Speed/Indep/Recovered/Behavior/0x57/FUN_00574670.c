// Original UG2 PC v1.2 0x574670, 103 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int FUN_00573700();
int FUN_005744c0();
int FUN_005744f0();
int FUN_00574530();


undefined4 FUN_00574670(uint *param_1,int param_2)

{
  uint uVar1;
  undefined4 uVar2;
  
  if ((param_1 != (uint *)0x0) && (param_2 != 0)) {
    uVar1 = *param_1;
    if (uVar1 < 0x57574153) {
      if (uVar1 == 0x57574152) {
        uVar2 = FUN_005744f0();
        return uVar2;
      }
      if (uVar1 == 0x46465548) {
        uVar2 = FUN_005744c0();
        return uVar2;
      }
      if (uVar1 == 0x504d4f43) {
        uVar2 = FUN_00574530();
        return uVar2;
      }
    }
    else if (uVar1 == 0x5a4c444a) {
      uVar2 = FUN_00573700();
      return uVar2;
    }
  }
  return 0;
}


