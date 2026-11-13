// Original UG2 PC v1.2 0x5df720, 236 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __stdcall FUN_005df630(int,int);
#define DAT_00800dfc (*(undefined4 *)0x00800dfc)

float10 __fastcall FUN_005df720(int param_1,int _edx_unused,int *param_2)

{
  int iVar1;
  char cVar2;
  int iVar3;
  int *piVar4;
  float10 fVar5;
  float local_10;
  int *local_c;
  int local_8;
  int local_4;
  
  local_10 = 99.0;
  local_4 = 0;
  local_8 = 0;
  if (*(int *)(param_1 + 0x298) < 1) {
    *param_2 = 0;
    return (float10)99.0;
  }
  local_c = (int *)(param_1 + 0x29c);
  do {
    iVar1 = *local_c;
    iVar3 = (int)*(short *)(iVar1 + 8) % 100;
    cVar2 = (char)((int)*(short *)(iVar1 + 8) / 100) + '@';
    if ((((('@' < cVar2) && (cVar2 < 'U')) && (DAT_00800dfc <= iVar3)) &&
        ((iVar3 < DAT_00800dfc * 2 && (*(int *)(iVar1 + 0xc) != 3)))) &&
       (*(int *)(iVar1 + 0xc) != 4)) {
      piVar4 = (int *)(param_1 + 0xb0);
      iVar3 = 2;
      do {
        if ((-1 < *piVar4) && (fVar5 = FUN_005df630(iVar1,(int)piVar4), fVar5 < (float10)local_10))
        {
          local_10 = (float)fVar5;
          local_4 = iVar1;
        }
        piVar4 = piVar4 + 0x17;
        iVar3 = iVar3 + -1;
      } while (iVar3 != 0);
    }
    local_8 = local_8 + 1;
    local_c = local_c + 1;
  } while (local_8 < *(int *)(param_1 + 0x298));
  *param_2 = local_4;
  return (float10)local_10;
}


