// Original UG2 PC v1.2 0x71ee1a, 175 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0071eb18();
int __cdecl FUN_0071eb7e();
int __cdecl FUN_0071ebe2();
#define PTR_DAT_00817b00 (*(undefined4 *)0x00817b00)

void FUN_0071ee1a(undefined1 (*param_1) [16],undefined1 (*param_2) [16],undefined8 *param_3,
                 int param_4,int param_5,int param_6)

{
  int iVar1;
  
  iVar1 = (int)param_2 - (int)param_1;
  if (iVar1 < 0) {
    iVar1 = (int)param_1 - (int)param_2;
    param_1 = param_2;
  }
  if (iVar1 == 1) {
    FUN_0071eb18(param_1,param_3,param_4,1,8,8,(short *)(&PTR_DAT_00817b00 + param_5 * 8));
    return;
  }
  if (iVar1 == param_4) {
    FUN_0071eb7e(param_1,param_3,param_4,param_4,8,8,(short *)(&PTR_DAT_00817b00 + param_6 * 8));
    return;
  }
  if (iVar1 == param_4 + -1) {
    param_1 = (undefined1 (*) [16])(param_1[-1] + 0xf);
  }
  else if (iVar1 != param_4 + 1) {
    return;
  }
  FUN_0071ebe2(param_1,param_3,param_4,(short *)(&PTR_DAT_00817b00 + param_5 * 8),
               (short *)(&PTR_DAT_00817b00 + param_6 * 8));
  return;
}


