// Original UG2 PC v1.2 0x5d5eb0, 283 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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

#define DAT_00800dfc (*(undefined4 *)0x00800dfc)

int __fastcall FUN_005d5eb0(void *_this,int _edx_unused,char param_1)

{
  short sVar1;
  int iVar2;
  char cVar3;
  int iVar4;
  int *piVar5;
  uint uVar6;
  uint local_c;
  int local_8;
  int local_4;
  
  local_8 = *(int *)((int)_this + 0x298);
  local_4 = 0;
  local_c = 0;
  if (local_8 < 1) {
    return 0;
  }
  piVar5 = (int *)((int)_this + 0x29c);
  do {
    iVar2 = *piVar5;
    uVar6 = 0;
    if (param_1 == '\0') {
      iVar4 = (int)*(short *)(iVar2 + 8) % 100;
      cVar3 = (char)((int)*(short *)(iVar2 + 8) / 100) + '@';
      if ((((cVar3 < 'A') || ('T' < cVar3)) || (iVar4 < DAT_00800dfc)) ||
         (DAT_00800dfc * 2 <= iVar4)) goto LAB_005d5f15;
    }
    else {
LAB_005d5f15:
      sVar1 = *(short *)(iVar2 + 8);
      iVar4 = (int)sVar1;
      cVar3 = (char)(iVar4 / 100) + '@';
      if ((cVar3 == 'Y') || (cVar3 == 'X')) {
        uVar6 = 2;
        if ((sVar1 == 0x9c4) || (sVar1 == 0x960)) {
          uVar6 = 1;
        }
      }
      else {
        if ((cVar3 < 'A') || ('T' < cVar3)) goto LAB_005d5f95;
        cVar3 = (char)(iVar4 / 100) + '@';
        if ((cVar3 < 'A') || (('T' < cVar3 || (iVar4 % 100 < DAT_00800dfc * 2)))) {
          uVar6 = 10;
        }
        else {
          uVar6 = 999;
        }
      }
      if ((*(int *)(iVar2 + 0xc) != 3) && (*(int *)(iVar2 + 0xc) != 4)) {
        uVar6 = uVar6 + 1;
      }
    }
LAB_005d5f95:
    if (local_c < uVar6) {
      local_c = uVar6;
      local_4 = iVar2;
    }
    piVar5 = piVar5 + 1;
    local_8 = local_8 + -1;
    if (local_8 == 0) {
      return local_4;
    }
  } while( true );
}


