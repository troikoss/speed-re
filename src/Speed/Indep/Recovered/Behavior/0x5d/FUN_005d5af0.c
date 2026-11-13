// Original UG2 PC v1.2 0x5d5af0, 133 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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



int __fastcall FUN_005d5af0(void *_this,int _edx_unused,float param_1,int param_2)

{
  int iVar1;
  int iVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  float local_4;
  
  iVar1 = *(int *)(param_2 * 0x18 + 0x34 + (int)_this);
  iVar5 = 0;
  iVar6 = 0;
  local_4 = 9999999.0;
  if (0 < iVar1) {
    do {
      iVar2 = *(int *)((int)_this + (*(int *)(param_2 * 0x18 + 0x30 + (int)_this) + iVar6) * 4 + 0x1b4
                      );
      fVar3 = param_1 - *(float *)(iVar2 + 0x84);
      fVar4 = *(float *)(iVar2 + 0x80) - param_1;
      if (fVar3 < fVar4) {
        fVar3 = fVar4;
      }
      if (fVar3 < local_4) {
        iVar5 = iVar2;
        local_4 = fVar3;
      }
      iVar6 = iVar6 + 1;
    } while (iVar6 < iVar1);
  }
  return iVar5;
}


