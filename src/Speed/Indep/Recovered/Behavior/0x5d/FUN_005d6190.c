// Original UG2 PC v1.2 0x5d6190, 124 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
#define DAT_007fb708 (*(undefined4 *)0x007fb708)

int __fastcall FUN_005d6190(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int iVar2;
  char cVar3;
  int iVar4;
  int *piVar5;
  int iVar6;
  
  iVar1 = *(int *)((int)_this + 0x298);
  if ((*(int *)((int)_this + 0x24) < iVar1) && (iVar6 = 0, 0 < iVar1)) {
    piVar5 = (int *)((int)_this + 0x29c);
    do {
      iVar2 = *piVar5;
      if (*(int *)(iVar2 + 0xc) == 3) {
        iVar4 = (int)*(short *)(iVar2 + 8) % 100;
        cVar3 = (char)((int)*(short *)(iVar2 + 8) / 100) + '@';
        if (((('@' < cVar3) && (cVar3 < 'U')) && (DAT_00800dfc <= iVar4)) &&
           ((iVar4 < DAT_00800dfc * 2 && (param_1 <= DAT_007fb708 - *(int *)(iVar2 + 0x3c))))) {
          return (int)*(short *)(iVar2 + 8);
        }
      }
      iVar6 = iVar6 + 1;
      piVar5 = piVar5 + 1;
    } while (iVar6 < iVar1);
  }
  return 0;
}


