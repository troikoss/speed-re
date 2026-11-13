// Original UG2 PC v1.2 0x722f98, 56 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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
int __cdecl FUN_00721f99();
int __cdecl FUN_00721fa7();
int __cdecl FUN_00727a12();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)

undefined4 FUN_00722f98(uint param_1,short param_2)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  uVar2 = 0xfffffff8;
  if (DAT_008b4608 == '\0') {
    return 0xfffffff6;
  }
  FUN_00721f99();
  puVar1 = FUN_00727a12(param_1);
  if (puVar1 != (undefined4 *)0x0) {
    puVar1[8] = (int)param_2;
    uVar2 = 0;
  }
  FUN_00721fa7();
  return uVar2;
}


