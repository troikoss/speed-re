// Original UG2 PC v1.2 0x727a12, 52 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
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


undefined4 * FUN_00727a12(uint param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (-1 < (int)param_1) {
    iVar1 = FUN_00722424(param_1 & 0xff);
    if (iVar1 != 0) {
      for (puVar2 = *(undefined4 **)(iVar1 + 0x144);
          (puVar2 != (undefined4 *)0x0 && (puVar2[3] != param_1)); puVar2 = (undefined4 *)*puVar2) {
      }
      return puVar2;
    }
  }
  return (undefined4 *)0x0;
}


