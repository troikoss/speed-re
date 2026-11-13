// Original UG2 PC v1.2 0x7230a7, 67 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00722424();
int __cdecl FUN_00723d9b();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)

undefined4 FUN_007230a7(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  if (DAT_008b4608 == '\0') {
    uVar2 = 0xfffffff6;
  }
  else {
    iVar1 = FUN_00722424(param_1);
    if (iVar1 != 0) {
      if (0x4000 < param_2) {
        param_2 = 0x4000;
      }
      *(short *)(iVar1 + 0x11c) = (short)param_2;
      FUN_00723d9b(*(uint *)(iVar1 + 4),param_2);
      return 0;
    }
    uVar2 = 0xfffffff8;
  }
  return uVar2;
}


