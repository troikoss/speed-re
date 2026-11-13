// Original UG2 PC v1.2 0x73b300, 123 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008b5744 (*(undefined4 *)0x008b5744)
#define DAT_008b5740 (*(undefined4 *)0x008b5740)
#define DAT_008b5768 (*(undefined4 *)0x008b5768)

undefined4 FUN_0073b300(uint param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  
  iVar3 = 0;
  uVar2 = 0;
  do {
    if (((*(int *)((int)&DAT_008b5744 + uVar2) != 0) &&
        ((*(uint *)((int)&DAT_008b5740 + uVar2) & param_1 & 0xf0000000) != 0)) &&
       ((*(uint *)((int)&DAT_008b5740 + uVar2) & param_1 & 0xf000000) != 0)) {
      uVar1 = 0;
      do {
        if ((param_1 & 0xffffff & 1 << ((byte)uVar1 & 0x1f)) != 0) {
          return (&DAT_008b5768)[iVar3 * 0x24a + uVar1];
        }
        uVar1 = uVar1 + 1;
      } while (uVar1 < 0x18);
    }
    uVar2 = uVar2 + 0x928;
    iVar3 = iVar3 + 1;
  } while (uVar2 < 0x24a0);
  return 0;
}


