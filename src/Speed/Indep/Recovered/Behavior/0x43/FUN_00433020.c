// Original UG2 PC v1.2 0x433020, 77 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int FUN_00431220();
int __cdecl FUN_0075c6c8();
#define PTR_FUN_00785fd8 (*(code *)0x00785fd8)

undefined4 * FUN_00433020(int param_1)

{
  undefined4 *puVar1;
  void *pvVar2;
  
  if (param_1 == 1) {
    pvVar2 = FUN_0075c6c8(0x38);
    if (pvVar2 != (void *)0x0) {
      puVar1 = (undefined4 *)FUN_00431220();
      return puVar1;
    }
  }
  else if (param_1 == 6) {
    puVar1 = FUN_0075c6c8(0x18);
    if (puVar1 != (undefined4 *)0x0) {
      *puVar1 = &PTR_FUN_00785fd8;
      puVar1[1] = 0;
      puVar1[2] = 0;
      puVar1[3] = 0;
      puVar1[4] = 0;
      *(undefined1 *)(puVar1 + 5) = 1;
      return puVar1;
    }
  }
  return (undefined4 *)0x0;
}


