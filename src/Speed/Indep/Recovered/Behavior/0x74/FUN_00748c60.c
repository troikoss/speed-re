// Original UG2 PC v1.2 0x748c60, 144 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_0075c784();
int __cdecl FUN_0075f1d0();
#define s_____CMD_http_URL_GET__s_____007d2bec ((char *)0x007d2bec)
#define s_http____007d2c14 ((char *)0x007d2c14)
#define s_https____007d2c08 ((char *)0x007d2c08)

void FUN_00748c60(undefined4 *param_1,char *param_2)

{
  char cVar1;
  int iVar2;
  char *pcVar3;
  
  *(undefined1 *)*param_1 = 0;
  param_1[0x144] = 0;
  iVar2 = FUN_0075f1d0(param_2,s_http____007d2c14,7);
  if (iVar2 != 0) {
    iVar2 = FUN_0075f1d0(param_2,s_https____007d2c08,8);
    if (iVar2 != 0) goto LAB_00748cb4;
  }
  FUN_0075c784((char *)*param_1,s_____CMD_http_URL_GET__s_____007d2bec,(int)param_2);
  param_2 = (char *)*param_1;
LAB_00748cb4:
  pcVar3 = param_2;
  do {
    cVar1 = *pcVar3;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  if ((uint)((int)pcVar3 - (int)(param_2 + 1)) < (uint)param_1[1]) {
    pcVar3 = (char *)*param_1;
    do {
      cVar1 = *param_2;
      param_2 = param_2 + 1;
      *pcVar3 = cVar1;
      pcVar3 = pcVar3 + 1;
    } while (cVar1 != '\0');
    param_1[0x144] = *param_1;
  }
  param_1[2] = 0;
  return;
}


