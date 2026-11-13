// Original UG2 PC v1.2 0x4c9170, 159 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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
int __cdecl FUN_0051de30();
#define DAT_0086518c (*(undefined4 *)0x0086518c)

void __fastcall FUN_004c9170(void *_this,int _edx_unused,char param_1,undefined4 param_2)

{
  undefined1 *puVar1;
  int iVar2;
  char *pcVar3;
  char *pcVar4;
  
  pcVar4 = (char *)((int)_this + 0x60);
  iVar2 = 0;
  pcVar3 = pcVar4;
  while ((pcVar3[1] != param_1 || (*pcVar3 == '\0'))) {
    iVar2 = iVar2 + 1;
    pcVar3 = pcVar3 + 0x14;
    if (7 < iVar2) {
LAB_004c91a0:
      iVar2 = 0;
      do {
        if (*pcVar4 == '\0') {
          puVar1 = (undefined1 *)((int)_this + iVar2 * 0x14 + 0x60);
          puVar1[1] = param_1;
          *puVar1 = 1;
          *(undefined4 *)(puVar1 + 0x10) = param_2;
          return;
        }
        iVar2 = iVar2 + 1;
        pcVar4 = pcVar4 + 0x14;
      } while (iVar2 < 8);
      return;
    }
  }
  pcVar3 = (char *)((int)_this + iVar2 * 0x14 + 0x60);
  if (pcVar3 != (char *)0x0) {
    if (*pcVar3 == '\x04') {
      pcVar3[0xc] = '\x04';
      *pcVar3 = '\x02';
      *(undefined4 *)(pcVar3 + 4) = DAT_0086518c;
      if (pcVar3[1] != '\x14') {
        if (pcVar3[1] == '\x13') {
          iVar2 = 0xf;
        }
        else {
          iVar2 = 0xd;
        }
        FUN_0051de30(0xb,iVar2);
      }
      *(undefined4 *)(pcVar3 + 0x10) = param_2;
    }
    return;
  }
  goto LAB_004c91a0;
}


