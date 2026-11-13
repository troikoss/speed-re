// Original UG2 PC v1.2 0x488c50, 130 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



int FUN_00488c50(byte param_1,int param_2,int param_3,int param_4,uint param_5)

{
  int iVar1;
  byte *pbVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  if (((param_2 != 0) && (0 < param_3)) && (param_4 + 1U <= param_5)) {
    iVar3 = 0;
    iVar4 = param_3 + -1;
    if (-1 < iVar4) {
      do {
        iVar5 = (iVar3 + iVar4) / 2;
        iVar1 = iVar5 * param_5 + param_2;
        if (param_1 < *(byte *)(iVar1 + param_4)) {
          iVar4 = iVar5 + -1;
        }
        else {
          if (param_1 <= *(byte *)(iVar1 + param_4)) {
            if (iVar5 < 1) {
              return iVar1;
            }
            pbVar2 = (byte *)((iVar1 - param_5) + param_4);
            do {
              if (*pbVar2 != param_1) {
                return iVar1;
              }
              iVar1 = iVar1 - param_5;
              pbVar2 = pbVar2 + -param_5;
              iVar5 = iVar5 + -1;
            } while (0 < iVar5);
            return iVar1;
          }
          iVar3 = iVar5 + 1;
        }
      } while (iVar3 <= iVar4);
    }
  }
  return 0;
}


