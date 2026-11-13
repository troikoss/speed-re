// Original UG2 PC v1.2 0x6da660, 116 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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



uint * FUN_006da660(int param_1,uint *param_2)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  
  puVar1 = (uint *)0x0;
  if (((param_2 != (uint *)0x0) && (-1 < param_1)) && (iVar3 = 0, puVar1 = param_2, param_1 != 0)) {
    do {
      uVar2 = *param_2 >> 0x10;
      if (((((uVar2 == 0x11) || (uVar2 == 0x10)) ||
           ((uVar2 == 0x41 || ((uVar2 == 0x40 || (uVar2 == 0x30)))))) || (uVar2 == 0x2f)) ||
         ((uVar2 == 0x43 || (uVar2 == 0x42)))) {
        param_2 = param_2 + (*param_2 & 0xffff);
        iVar3 = iVar3 + 1;
      }
      uVar2 = *param_2;
      if (uVar2 == 0) {
        param_2 = param_2 + 1;
      }
      puVar1 = param_2 + (uVar2 & 0xffff);
      param_2 = puVar1;
    } while (iVar3 != param_1);
  }
  return puVar1;
}


