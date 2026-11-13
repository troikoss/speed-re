// Original UG2 PC v1.2 0x642e31, 270 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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



undefined4 __fastcall FUN_00642e31(void *_this,int _edx_unused,char *param_1,char *param_2)

{
  char *pcVar1;
  char cVar2;
  char cVar3;
  
  param_2[0] = '\0';
  param_2[1] = '\0';
  param_2[2] = '\0';
  param_2[3] = '\0';
  *param_2 = *param_1;
  pcVar1 = param_1 + 1;
  if (*(char **)((int)_this + 4) <= pcVar1) {
    return 1;
  }
  cVar2 = *param_1;
  if ((cVar2 == '#') && ((cVar3 = *pcVar1, cVar3 == '#' || (cVar3 == '@')))) {
    param_2[1] = cVar3;
    return 2;
  }
  cVar3 = *pcVar1;
  if (cVar2 != cVar3) {
    if (cVar3 == '=') {
      if (cVar2 < '0') {
        if (((cVar2 != '/') && (cVar2 != '!')) &&
           ((cVar2 < '%' || (('&' < cVar2 && ((cVar2 < '*' || (('+' < cVar2 && (cVar2 != '-'))))))))
           )) {
          return 1;
        }
      }
      else if ((((cVar2 != '<') && (cVar2 != '>')) && (cVar2 != '^')) && (cVar2 != '|')) {
        return 1;
      }
      param_2[1] = '=';
      return 2;
    }
    if (cVar2 != '-') {
      return 1;
    }
    if (cVar3 == '>') {
      param_2[1] = '>';
      return 2;
    }
    return 1;
  }
  if (cVar2 < ';') {
    if ((((cVar2 != ':') && (cVar2 != '&')) && (cVar2 != '+')) && (cVar2 != '-')) {
      if (cVar2 != '.') {
        return 1;
      }
      pcVar1 = param_1 + 2;
      if (*(char **)((int)_this + 4) <= pcVar1) {
        return 1;
      }
      if (*pcVar1 == '.') {
        param_2[1] = cVar3;
        param_2[2] = *pcVar1;
        return 3;
      }
      return 1;
    }
  }
  else {
    if (cVar2 == '<') {
LAB_00642ec2:
      param_2[1] = cVar3;
      if (*(char **)((int)_this + 4) <= param_1 + 2) {
        return 2;
      }
      if (param_1[2] == '=') {
        param_2[2] = '=';
        return 3;
      }
      return 2;
    }
    if (cVar2 != '=') {
      if (cVar2 == '>') goto LAB_00642ec2;
      if (cVar2 != '|') {
        return 1;
      }
    }
  }
  param_2[1] = cVar3;
  return 2;
}


