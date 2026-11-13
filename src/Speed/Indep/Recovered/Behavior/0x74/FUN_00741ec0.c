// Original UG2 PC v1.2 0x741ec0, 89 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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
int __cdecl FUN_00741270();


int FUN_00741ec0(byte *param_1,int param_2,byte *param_3)

{
  byte *pbVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = 0;
  if (*param_3 == 0) {
    return 0;
  }
  do {
    bVar2 = *param_3;
    if (bVar2 < 0x21) {
      param_3 = param_3 + 1;
    }
    else {
      if (bVar2 == 0x3d) {
        return iVar4;
      }
      if (bVar2 == 0x3a) {
        return iVar4;
      }
      iVar3 = FUN_00741270(param_1,param_2,param_3);
      if (0 < iVar3) {
        iVar4 = iVar4 + 1;
      }
      bVar2 = *param_3;
      while (0x1f < bVar2) {
        pbVar1 = param_3 + 1;
        param_3 = param_3 + 1;
        bVar2 = *pbVar1;
      }
    }
    if (*param_3 == 0) {
      return iVar4;
    }
  } while( true );
}


