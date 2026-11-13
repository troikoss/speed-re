// Original UG2 PC v1.2 0x7431e0, 224 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



byte FUN_007431e0(uint *param_1,int *param_2,int *param_3,byte *param_4)

{
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  uint uVar4;
  byte bVar5;
  
  bVar1 = *param_4;
  while (('\0' < (char)bVar1 && ((char)bVar1 < '!'))) {
    pbVar2 = param_4 + 1;
    param_4 = param_4 + 1;
    bVar1 = *pbVar2;
  }
  uVar4 = 0;
  do {
    while( true ) {
      bVar1 = *param_4;
      if (('/' < (char)bVar1) && ((char)bVar1 < ':')) break;
      if (bVar1 != 0x2e) {
        bVar5 = uVar4 != 0;
        *param_1 = uVar4;
        bVar1 = *param_4;
        while ((bVar1 != 0x3a && (bVar1 != 0))) {
          pbVar2 = param_4 + 1;
          param_4 = param_4 + 1;
          bVar1 = *pbVar2;
        }
        iVar3 = 0;
        if (*param_4 == 0x3a) {
          param_4 = param_4 + 1;
          bVar1 = *param_4;
          while (('/' < (char)bVar1 && ((char)bVar1 < ':'))) {
            param_4 = param_4 + 1;
            iVar3 = (bVar1 & 0xf) + iVar3 * 10;
            bVar1 = *param_4;
          }
          bVar5 = bVar5 | 2;
        }
        *param_2 = iVar3;
        if (param_3 != (int *)0x0) {
          iVar3 = 0;
          if (*param_4 == 0x3a) {
            pbVar2 = param_4 + 1;
            bVar1 = *pbVar2;
            while (('/' < (char)bVar1 && ((char)bVar1 < ':'))) {
              pbVar2 = pbVar2 + 1;
              iVar3 = (bVar1 & 0xf) + iVar3 * 10;
              bVar1 = *pbVar2;
            }
            bVar5 = bVar5 | 4;
          }
          *param_3 = iVar3;
        }
        return bVar5;
      }
LAB_00743212:
      uVar4 = uVar4 << 8;
      param_4 = param_4 + 1;
    }
    if (bVar1 == 0x2e) goto LAB_00743212;
    uVar4 = uVar4 + (uVar4 & 0xff) * 9 + (bVar1 & 0xf);
    param_4 = param_4 + 1;
  } while( true );
}


