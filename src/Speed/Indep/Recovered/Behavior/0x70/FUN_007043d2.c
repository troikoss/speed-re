// Original UG2 PC v1.2 0x7043d2, 269 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00704078();
int __cdecl FUN_0070423c();
int __cdecl FUN_00704279();
int __cdecl FUN_0070439a();
#define DAT_00816708 (*(undefined4 *)0x00816708)
#define DAT_00816688 (*(undefined4 *)0x00816688)

void FUN_007043d2(byte *param_1,byte *param_2,int param_3,int param_4,int param_5,int param_6,
                 int param_7)

{
  int iVar1;
  
  iVar1 = (int)param_2 - (int)param_1;
  if (iVar1 < 0) {
    iVar1 = (int)param_1 - (int)param_2;
    param_1 = param_2;
  }
  if (iVar1 == 1) {
    if (param_7 == 0) {
      iVar1 = 1;
LAB_00704456:
      FUN_00704279(param_1,param_3,param_4,iVar1,8,8,(int *)(&DAT_00816708 + param_5 * 8));
      return;
    }
    iVar1 = 1;
  }
  else {
    if (iVar1 != param_4) {
      if (iVar1 == param_4 + -1) {
        if (param_7 != 0) {
          param_1 = param_1 + -1;
LAB_007044d2:
          FUN_0070423c((int)param_1,param_3,param_4,(int *)(&DAT_00816688 + param_5 * 0x10),
                       (int *)(&DAT_00816688 + param_6 * 0x10));
          return;
        }
        param_1 = param_1 + -1;
      }
      else {
        if (iVar1 != param_4 + 1) {
          return;
        }
        if (param_7 != 0) goto LAB_007044d2;
      }
      FUN_0070439a(param_1,param_3,param_4,(int *)(&DAT_00816708 + param_5 * 8),
                   (int *)(&DAT_00816708 + param_6 * 8));
      return;
    }
    param_5 = param_6;
    if (param_7 == 0) {
      iVar1 = param_4;
      goto LAB_00704456;
    }
    iVar1 = param_4;
  }
  FUN_00704078(param_1,param_3,param_4,iVar1,8,8,(int *)(&DAT_00816688 + param_5 * 0x10));
  return;
}


