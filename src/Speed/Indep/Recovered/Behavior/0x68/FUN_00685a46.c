// Original UG2 PC v1.2 0x685a46, 181 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int a;
typedef int b;
typedef int x;
typedef int y;

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
int __cdecl FUN_0064307c();
#define s_invalid_mask___s__007b9ef8 ((char *)0x007b9ef8)

uint __fastcall FUN_00685a46(void *_this,int _edx_unused,int param_1)

{
  char *pcVar1;
  uint uVar2;
  char cVar3;
  void *pvVar4;
  char *pcVar5;
  void *local_8;
  
  pcVar1 = *(char **)(param_1 + 8);
  uVar2 = 0;
  if ((pcVar1 == (char *)0x0) || (cVar3 = *pcVar1, cVar3 == '\0')) {
    uVar2 = 0xf0000;
  }
  else {
    pcVar5 = pcVar1;
    local_8 = _this;
    do {
      if (cVar3 < 'x') {
        if ((cVar3 == 'w') || (cVar3 == 'a')) {
          pvVar4 = (void *)0x3;
          uVar2 = uVar2 | 0x80000;
        }
        else {
          if (cVar3 != 'b') {
            if (cVar3 == 'g') goto LAB_00685ac3;
            if (cVar3 != 'r') goto LAB_00685acd;
            goto LAB_00685a88;
          }
LAB_00685ab9:
          pvVar4 = (void *)0x2;
          uVar2 = uVar2 | 0x40000;
        }
      }
      else if (cVar3 == 'x') {
LAB_00685a88:
        pvVar4 = (void *)0x0;
        uVar2 = uVar2 | 0x10000;
      }
      else {
        if (cVar3 != 'y') {
          if (cVar3 != 'z') goto LAB_00685acd;
          goto LAB_00685ab9;
        }
LAB_00685ac3:
        pvVar4 = (void *)0x1;
        uVar2 = uVar2 | 0x20000;
      }
      if ((pcVar5 != pcVar1) && (pvVar4 <= local_8)) {
LAB_00685acd:
        FUN_0064307c(*(void **)_this,param_1,0x7d3,s_invalid_mask___s__007b9ef8,(char)pcVar1);
        *(undefined4 *)((int)_this + 0x4c) = 1;
        return 0;
      }
      pcVar5 = pcVar5 + 1;
      cVar3 = *pcVar5;
      local_8 = pvVar4;
    } while (cVar3 != '\0');
  }
  return uVar2;
}


