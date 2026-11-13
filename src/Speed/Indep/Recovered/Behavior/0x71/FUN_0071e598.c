// Original UG2 PC v1.2 0x71e598, 175 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0071e495();
int __cdecl FUN_0071e51b();
int __cdecl FUN_0071e7f9();
#define PTR_DAT_008177d0 (*(undefined4 *)0x008177d0)

void FUN_0071e598(undefined8 *param_1,undefined8 *param_2,undefined4 *param_3,int param_4,
                 int param_5,int param_6)

{
  int iVar1;
  
  iVar1 = (int)param_2 - (int)param_1;
  if (iVar1 < 0) {
    iVar1 = (int)param_1 - (int)param_2;
    param_1 = param_2;
  }
  if (iVar1 == 1) {
    FUN_0071e495(param_1,param_3,param_4,1,8,8,(undefined8 *)(&PTR_DAT_008177d0 + param_5 * 8));
    return;
  }
  if (iVar1 == param_4) {
    FUN_0071e51b(param_1,param_3,param_4,param_4,8,8,(undefined8 *)(&PTR_DAT_008177d0 + param_6 * 8)
                );
    return;
  }
  if (iVar1 == param_4 + -1) {
    param_1 = (undefined8 *)((int)param_1 + -1);
  }
  else if (iVar1 != param_4 + 1) {
    return;
  }
  FUN_0071e7f9(param_1,(undefined8 *)param_3,param_4,(undefined8 *)(&PTR_DAT_008177d0 + param_5 * 8)
               ,(undefined8 *)(&PTR_DAT_008177d0 + param_6 * 8));
  return;
}


