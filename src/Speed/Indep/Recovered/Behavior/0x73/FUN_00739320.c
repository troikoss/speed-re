// Original UG2 PC v1.2 0x739320, 174 bytes. BEHAVIOUR-VERIFIED ONLY (39 clean trials, 1 both-fault):
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
int __cdecl FUN_0046c690();
int __cdecl FUN_0073b620();
#define DAT_008b7bbc (*(undefined4 *)0x008b7bbc)
#define DAT_008b7bc0 (*(undefined4 *)0x008b7bc0)
#define DAT_008b5740 (*(undefined4 *)0x008b5740)
#define DAT_008b7bb0 (*(undefined4 *)0x008b7bb0)

void FUN_00739320(uint param_1)

{
  int iVar1;
  int extraout_EAX;
  int extraout_EAX_00;
  int iVar2;
  uint *puVar3;
  int iVar4;
  
  if ((DAT_008b7bbc != '\0') && (DAT_008b7bc0 == 0)) {
    FUN_0073b620(-1);
    if (extraout_EAX != 0) {
      iVar2 = 0;
      puVar3 = &DAT_008b5740;
      do {
        if (((puVar3[1] != 0) && ((*puVar3 & param_1 & 0xf0000000) != 0)) &&
           ((*puVar3 & param_1 & 0xf000000) != 0)) {
          FUN_0073b620(iVar2);
          if (extraout_EAX_00 != 0) {
            iVar4 = 0x58;
            do {
              iVar1 = *(int *)(iVar4 + DAT_008b7bb0);
              if (iVar1 != 0) {
                FUN_0046c690(*(int *)(iVar1 + 0x54));
                *(undefined2 *)(iVar1 + 0x36) = 0xffff;
              }
              iVar4 = iVar4 + 4;
            } while (iVar4 < 0xb8);
          }
        }
        puVar3 = puVar3 + 0x24a;
        iVar2 = iVar2 + 1;
      } while ((int)puVar3 < 0x8b7be0);
      DAT_008b7bc0 = 0;
      DAT_008b7bb0 = 0;
    }
  }
  return;
}


