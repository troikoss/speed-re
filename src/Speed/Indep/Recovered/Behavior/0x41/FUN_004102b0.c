// Original UG2 PC v1.2 0x4102b0, 221 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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



void FUN_004102b0(undefined4 param_1,int param_2,char param_3,int param_4)

{
  int iVar1;
  bool bVar2;
  
  iVar1 = *(int *)(*(int *)(param_4 + 0x220) + 0x2c);
  bVar2 = param_3 != '\0';
  if ((param_2 == 0xb8) || (param_2 == 0xba)) {
    if ((bool)*(char *)(param_4 + 0x791) == bVar2) {
      return;
    }
    *(bool *)(param_4 + 0x791) = bVar2;
  }
  else {
    if ((param_2 != 0xb9) && (param_2 != 0xbb)) {
      return;
    }
    if ((bool)*(char *)(param_4 + 0x790) == bVar2) {
      return;
    }
    *(bool *)(param_4 + 0x790) = bVar2;
  }
  switch(param_2) {
  case 0xb8:
  case 0xba:
    if (param_3 != '\0') {
      if (*(int *)(iVar1 + 0x7b4) == 0) {
        return;
      }
      *(undefined4 *)(iVar1 + 0x7b4) = 0;
LAB_00410357:
      *(undefined4 *)(iVar1 + 0x7b8) = 0xff;
      *(undefined4 *)(iVar1 + 0x7bc) = 0xff;
      *(undefined1 *)(iVar1 + 0x7c0) = 0;
      return;
    }
    break;
  case 0xb9:
  case 0xbb:
    if (param_3 != '\0') {
      if (*(int *)(iVar1 + 0x7b4) == 1) {
        return;
      }
      *(undefined4 *)(iVar1 + 0x7b4) = 1;
      goto LAB_00410357;
    }
    break;
  default:
    goto switchD_0041031d_default;
  }
  *(undefined1 *)(iVar1 + 0x774) = 1;
  *(undefined4 *)(iVar1 + 0x7b4) = 2;
  *(undefined1 *)(iVar1 + 0x7c0) = 0;
switchD_0041031d_default:
  return;
}


