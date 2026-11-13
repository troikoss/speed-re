// Original UG2 PC v1.2 0x72194e, 66 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_007243e4();
int __cdecl FUN_00724459();
int __cdecl FUN_0075bed4();


undefined4 FUN_0072194e(int param_1)

{
  undefined4 uVar1;
  ulonglong uVar2;
  undefined4 local_d0 [16];
  undefined1 local_8d;
  
  if (param_1 == 0) {
    FUN_007243e4(local_d0);
    uVar2 = FUN_0075bed4();
    local_8d = (undefined1)uVar2;
    FUN_00724459((int)local_d0);
    uVar1 = 0;
  }
  else {
    uVar1 = 0xfffffffb;
  }
  return uVar1;
}


