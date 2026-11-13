// Original UG2 PC v1.2 0x65ee40, 48 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



void __fastcall FUN_0065ee40(int *param_1)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = *param_1;
  if (((iVar1 == 4) || ((4 < iVar1 && (((iVar1 < 10 || (iVar1 == 0xf)) || (iVar1 == 0x10)))))) &&
     (piVar2 = (int *)param_1[1], piVar2 != (int *)0x0)) {
    (**(code **)(*piVar2 + 8))(piVar2);
    param_1[1] = 0;
  }
  return;
}


