// Original UG2 PC v1.2 0x42fb60, 137 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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



void __fastcall FUN_0042fb60(void *_this,int _edx_unused,int param_1,char *param_2)

{
  int iVar1;
  char *pcVar2;
  char cVar3;
  int iVar4;
  int iVar5;
  char *pcVar6;
  
  pcVar2 = (char *)((*(uint *)((int)_this + param_1 * 8 + 8) >> 0x18) * 0x40 + 0x808 + (int)_this);
  iVar4 = 0;
  cVar3 = *pcVar2;
  while (cVar3 != '_') {
    iVar5 = iVar4 + 1;
    iVar4 = iVar4 + 1;
    cVar3 = pcVar2[iVar5];
  }
  iVar4 = iVar4 + 1;
  pcVar6 = pcVar2 + iVar4;
  cVar3 = pcVar2[iVar4];
  iVar5 = iVar4;
  while (cVar3 != '_') {
    iVar1 = iVar5 + 1;
    iVar5 = iVar5 + 1;
    cVar3 = pcVar2[iVar1];
  }
  iVar5 = iVar5 - iVar4;
  if (iVar5 == 0) {
    *param_2 = '\0';
    return;
  }
  do {
    *param_2 = *pcVar6;
    param_2 = param_2 + 1;
    pcVar6 = pcVar6 + 1;
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  *param_2 = '\0';
  return;
}


