// Original UG2 PC v1.2 0x7419a0, 164 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_007419a0(byte *param_1,int param_2,char *param_3,int param_4)

{
  byte *pbVar1;
  byte *pbVar2;
  char cVar3;
  byte local_124 [288];
  undefined4 local_4;
  
  pbVar1 = local_124;
  pbVar2 = local_124;
  local_4 = DAT_0081a1d0;
  if (param_3 == (char *)0x0) {
    *param_1 = 0;
  }
  else {
    cVar3 = *param_3;
    if (cVar3 != '\0') {
      pbVar1 = local_124;
      do {
        *pbVar1 = cVar3;
        cVar3 = pbVar1[(int)(param_3 + (1 - (int)local_124))];
        pbVar1 = pbVar1 + 1;
      } while (cVar3 != '\0');
    }
    *pbVar1 = '=';
    pbVar2 = pbVar1 + 1;
  }
  for (; param_4 != 0; param_4 = param_4 << 8) {
    if (0x20ffffff < param_4) {
      *pbVar2 = (char)((uint)param_4 >> 0x18);
      pbVar2 = pbVar2 + 1;
    }
  }
  *pbVar2 = '\0';
  FUN_00741270(param_1,param_2,local_124);
  return;
}


