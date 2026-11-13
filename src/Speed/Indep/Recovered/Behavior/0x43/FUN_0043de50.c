// Original UG2 PC v1.2 0x43de50, 152 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int x;

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



int FUN_0043de50(char *param_1)

{
  char cVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  byte bVar5;
  int iVar6;
  char *pcVar7;
  
  cVar1 = *param_1;
  if ((cVar1 == '0') && (param_1[1] == 'x')) {
    bVar5 = param_1[2];
    pcVar7 = param_1 + 2;
    iVar4 = 0;
    if (bVar5 == 0) {
      return 0;
    }
    do {
      if (((char)bVar5 < '0') || ('9' < (char)bVar5)) {
        if (('`' < (char)bVar5) && ((char)bVar5 < '{')) {
          bVar5 = bVar5 & 0x5f;
        }
        if ((char)bVar5 < 'A') {
          return iVar4;
        }
        if ('F' < (char)bVar5) {
          return iVar4;
        }
        iVar6 = (char)bVar5 + -0x37;
      }
      else {
        iVar6 = (char)bVar5 + -0x30;
      }
      iVar4 = iVar4 * 0x10 + iVar6;
      bVar5 = pcVar7[1];
      pcVar7 = pcVar7 + 1;
      if (bVar5 == 0) {
        return iVar4;
      }
    } while( true );
  }
  bVar2 = false;
  if (cVar1 == '-') {
    bVar3 = true;
  }
  else {
    bVar3 = false;
    if (cVar1 != '+') goto LAB_0043debd;
  }
  bVar2 = bVar3;
  param_1 = param_1 + 1;
LAB_0043debd:
  iVar4 = 0;
  cVar1 = *param_1;
  while (((cVar1 != '\0' && ('/' < cVar1)) && (cVar1 < ':'))) {
    param_1 = param_1 + 1;
    iVar4 = cVar1 + -0x30 + iVar4 * 10;
    cVar1 = *param_1;
  }
  if (bVar2) {
    iVar4 = -iVar4;
  }
  return iVar4;
}


