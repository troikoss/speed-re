// Original UG2 PC v1.2 0x45a9b0, 93 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_00440540();
int __cdecl FUN_00736260();


void __fastcall FUN_0045a9b0(int param_1)

{
  undefined4 *puVar1;
  
  if (*(char *)(param_1 + 0x14) != '\0') {
    FUN_00736260();
    puVar1 = *(undefined4 **)(param_1 + 4);
    if (*(int *)(param_1 + 0xc) == 1) {
      if (puVar1 != (undefined4 *)0x0) {
        (**(code **)*puVar1)(1);
      }
      if (*(undefined4 **)(param_1 + 8) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 8))(1);
      }
    }
    else if (puVar1 != (undefined4 *)0x0) {
      (**(code **)*puVar1)(1);
    }
    *(undefined4 *)(param_1 + 4) = 0;
    *(undefined4 *)(param_1 + 8) = 0;
    if (*(void **)(param_1 + 0x1c) != (void *)0x0) {
      FUN_00440540(*(void **)(param_1 + 0x1c));
      *(undefined4 *)(param_1 + 0x1c) = 0;
    }
    *(undefined1 *)(param_1 + 0x14) = 0;
  }
  return;
}


