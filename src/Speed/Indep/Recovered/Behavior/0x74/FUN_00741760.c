// Original UG2 PC v1.2 0x741760, 163 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define s__ABCDEFGHIJKLMNOPQRSTUVWXYZ0123_007d2698 ((char *)0x007d2698)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_00741760(byte *param_1,int param_2,byte *param_3,uint param_4)

{
  byte *pbVar1;
  byte bVar2;
  char *pcVar3;
  byte local_124 [288];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  pbVar1 = local_124;
  if (param_3 == (byte *)0x0) {
    *param_1 = 0;
  }
  else {
    bVar2 = *param_3;
    if (bVar2 != 0) {
      do {
        *pbVar1 = bVar2;
        bVar2 = pbVar1[(int)(param_3 + (1 - (int)local_124))];
        pbVar1 = pbVar1 + 1;
      } while (bVar2 != 0);
    }
    *pbVar1 = 0x3d;
    pbVar1 = pbVar1 + 1;
  }
  for (pcVar3 = s__ABCDEFGHIJKLMNOPQRSTUVWXYZ0123_007d2698; (param_4 != 0 && (*pcVar3 != 0));
      pcVar3 = pcVar3 + 1) {
    if ((param_4 & 1) != 0) {
      *pbVar1 = *pcVar3;
      pbVar1 = pbVar1 + 1;
    }
    param_4 = (int)param_4 >> 1;
  }
  *pbVar1 = 0;
  FUN_00741270(param_1,param_2,local_124);
  return;
}


