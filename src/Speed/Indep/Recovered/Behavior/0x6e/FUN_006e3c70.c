// Original UG2 PC v1.2 0x6e3c70, 529 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



void __fastcall FUN_006e3c70(void *_this,int _edx_unused,undefined4 *param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = *(int *)((int)_this + 8);
  if (param_2 == 0) {
    if (*(int *)((int)_this + 0xc) == 0) {
      if (0 < iVar3) {
        puVar1 = (undefined4 *)((int)_this + 0x18);
        do {
          *param_1 = puVar1[-2];
          param_1[1] = puVar1[-1];
          param_1[2] = *puVar1;
          param_1[3] = 0x3f800000;
          param_1[4] = puVar1[2];
          param_1[5] = puVar1[3];
          param_1[6] = puVar1[4];
          param_1[7] = puVar1[5];
          param_1[8] = puVar1[6];
          param_1[9] = puVar1[7];
          param_1[10] = puVar1[8];
          param_1[0xb] = 0x3f800000;
          param_1 = param_1 + 0xc;
          puVar1 = puVar1 + 0x1c;
          iVar3 = iVar3 + -1;
        } while (iVar3 != 0);
      }
    }
    else {
      iVar2 = 0;
      if (0 < iVar3) {
        puVar1 = (undefined4 *)((int)_this + 0x18);
        do {
          *param_1 = puVar1[-2];
          param_1[1] = puVar1[-1];
          param_1[2] = *puVar1;
          param_1[3] = *(undefined4 *)(*(int *)((int)_this + 0xc) + iVar2 * 4);
          param_1[4] = puVar1[2];
          param_1[5] = puVar1[3];
          param_1[6] = puVar1[4];
          param_1[7] = puVar1[5];
          param_1[8] = puVar1[6];
          param_1[9] = puVar1[7];
          param_1[10] = puVar1[8];
          param_1[0xb] = 0x3f800000;
          param_1 = param_1 + 0xc;
          iVar2 = iVar2 + 1;
          puVar1 = puVar1 + 0x1c;
        } while (iVar2 < iVar3);
        return;
      }
    }
  }
  else {
    iVar2 = 0;
    if (*(int *)((int)_this + 0xc) == 0) {
      if (0 < iVar3) {
        puVar1 = (undefined4 *)((int)_this + 0x18);
        do {
          if ((*(uint *)(param_2 + (iVar2 >> 5) * 4) & 1 << ((byte)iVar2 & 0x1f)) != 0) {
            *param_1 = puVar1[-2];
            param_1[1] = puVar1[-1];
            param_1[2] = *puVar1;
            param_1[3] = 0x3f800000;
            param_1[4] = puVar1[2];
            param_1[5] = puVar1[3];
            param_1[6] = puVar1[4];
            param_1[7] = puVar1[5];
            param_1[8] = puVar1[6];
            param_1[9] = puVar1[7];
            param_1[10] = puVar1[8];
            param_1[0xb] = 0x3f800000;
          }
          param_1 = param_1 + 0xc;
          iVar2 = iVar2 + 1;
          puVar1 = puVar1 + 0x1c;
        } while (iVar2 < iVar3);
        return;
      }
    }
    else if (0 < iVar3) {
      puVar1 = (undefined4 *)((int)_this + 0x18);
      do {
        if ((*(uint *)(param_2 + (iVar2 >> 5) * 4) & 1 << ((byte)iVar2 & 0x1f)) != 0) {
          *param_1 = puVar1[-2];
          param_1[1] = puVar1[-1];
          param_1[2] = *puVar1;
          param_1[3] = *(undefined4 *)(*(int *)((int)_this + 0xc) + iVar2 * 4);
          param_1[4] = puVar1[2];
          param_1[5] = puVar1[3];
          param_1[6] = puVar1[4];
          param_1[7] = puVar1[5];
          param_1[8] = puVar1[6];
          param_1[9] = puVar1[7];
          param_1[10] = puVar1[8];
          param_1[0xb] = 0x3f800000;
        }
        param_1 = param_1 + 0xc;
        iVar2 = iVar2 + 1;
        puVar1 = puVar1 + 0x1c;
      } while (iVar2 < iVar3);
      return;
    }
  }
  return;
}


